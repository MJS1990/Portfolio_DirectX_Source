#include "Framework.h"
#include "Shader.h"

Shader::Shader(wstring file, bool bSubContext)
	: file(file)
{
	initialStateBlock = new StateBlock();
	{	
		D3D::GetDC()->RSGetState(&initialStateBlock->RSRasterizerState);
		D3D::GetDC()->OMGetBlendState(&initialStateBlock->OMBlendState, initialStateBlock->OMBlendFactor, &initialStateBlock->OMSampleMask);
		D3D::GetDC()->OMGetDepthStencilState(&initialStateBlock->OMDepthStencilState, &initialStateBlock->OMStencilRef);
	}

	CreateEffect();

	if (bSubContext == false)
		Context::Get()->AddShader(this);
}

Shader::~Shader()
{
	for (Technique& temp : techniques)
	{
		for (Pass& pass : temp.Passes)
		{
			SAFE_RELEASE(pass.InputLayout);
		}
	}

	SAFE_DELETE(initialStateBlock);
	SAFE_RELEASE(effect);
}

void Shader::CreateEffect()
{
	ID3DBlob* fxBlob; //�����Ͻ� �� ���������ο� ���� ���� �ڵ带 �����ϴ� ����

	Shaders::GetEffect(file, &fxBlob, &effect);

	effect->GetDesc(&effectDesc);
	for (UINT t = 0; t < effectDesc.Techniques; t++)
	{
		Technique technique;
		technique.ITechnique = effect->GetTechniqueByIndex(t);
		technique.ITechnique->GetDesc(&technique.Desc);
		technique.Name = String::ToWString(technique.Desc.Name);

		for (UINT p = 0; p < technique.Desc.Passes; p++)
		{
			Pass pass;
			pass.IPass = technique.ITechnique->GetPassByIndex(p);
			pass.IPass->GetDesc(&pass.Desc);
			pass.Name = String::ToWString(pass.Desc.Name);
			pass.IPass->GetVertexShaderDesc(&pass.PassVsDesc);
			pass.PassVsDesc.pShaderVariable->GetShaderDesc(pass.PassVsDesc.ShaderIndex, &pass.EffectVsDesc);

			for (UINT s = 0; s < pass.EffectVsDesc.NumInputSignatureEntries; s++)
			{
				D3D11_SIGNATURE_PARAMETER_DESC desc;

				HRESULT hr = pass.PassVsDesc.pShaderVariable->GetInputSignatureElementDesc(pass.PassVsDesc.ShaderIndex, s, &desc);
				assert(SUCCEEDED(hr));

				pass.SignatureDescs.push_back(desc);
			}

			pass.InputLayout = CreateInputLayout(fxBlob, &pass.EffectVsDesc, pass.SignatureDescs);
			pass.StateBlock = initialStateBlock;

			technique.Passes.push_back(pass);
		}

		techniques.push_back(technique);
	}

	//TODO : Create ConstantBuffers, GloablVariables
	//TODO : SAFE_RESLEASE(fxBlob) �ּ�
	//SAFE_RELEASE(fxBlob);
}

ID3D11InputLayout * Shader::CreateInputLayout(ID3DBlob * fxBlob, D3DX11_EFFECT_SHADER_DESC * effectVsDesc, vector<D3D11_SIGNATURE_PARAMETER_DESC>& params)
{
	//TODO : std::
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;

	for (D3D11_SIGNATURE_PARAMETER_DESC& paramDesc : params)
	{
		//InputAssmbler�ܰ迡�� ���� ������Ʈ Desc
		D3D11_INPUT_ELEMENT_DESC elementDesc;
		elementDesc.SemanticName = paramDesc.SemanticName;
		elementDesc.SemanticIndex = paramDesc.SemanticIndex;
		elementDesc.InputSlot = 0;
		elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT; //������ �ڵ�
		elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elementDesc.InstanceDataStepRate = 0;

		if (paramDesc.Mask == 1)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
				elementDesc.Format = DXGI_FORMAT_R32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
				elementDesc.Format = DXGI_FORMAT_R32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
				elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
		}
		else if (paramDesc.Mask <= 3)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
				elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
				elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
				elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		}
		else if (paramDesc.Mask <= 7)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
				elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
				elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
				elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		}
		else if (paramDesc.Mask <= 15)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32)
				elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32)
				elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32)
				elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		}
	
		string name = paramDesc.SemanticName;
		transform(name.begin(), name.end(), name.begin(), toupper); //name �빮�� ������ ����
		
		if (name == "POSITION")
		{
			elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			//elementDesc.InputSlot = paramDesc.SematicIndex;
		}

		if (name == "INSTANCE")
		{
			elementDesc.InputSlot = 1;
			elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			elementDesc.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
			elementDesc.InstanceDataStepRate = 1;
		}

		inputLayoutDesc.push_back(elementDesc);
	}

	const void* pCode = effectVsDesc->pBytecode;
	UINT pCodeSize = effectVsDesc->BytecodeLength;

	if (inputLayoutDesc.size() > 0) //���۰� �����ϸ� InputLayout����
	{
		ID3D11InputLayout* inputLayout = NULL;
		HRESULT hr = D3D::GetDevice()->CreateInputLayout
		(
			&inputLayoutDesc[0]
			, inputLayoutDesc.size()
			, pCode
			, pCodeSize
			, &inputLayout
		);
		assert(SUCCEEDED(hr));

		return inputLayout;
	}
	return NULL;
}
//Pass Draw////////////////////////////////////////////////////////////////////
void Shader::Pass::Draw(UINT vertexCount, UINT startVertexLocation)
{
	BeginDraw();
	{
		D3D::GetDC()->Draw(vertexCount, startVertexLocation);
	}
	EndDraw();
}

void Shader::Pass::DrawIndexed(UINT indexCount, UINT startIndexLocation, INT baseVertexLocation)
{
	BeginDraw();
	{
		D3D::GetDC()->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
	}
	EndDraw();
}

void Shader::Pass::DrawInstanced(UINT vertexCountPerInstance, UINT instanceCount, UINT startVertexLocation, UINT startInstanceLocation)
{
	BeginDraw();
	{
		D3D::GetDC()->DrawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
	}
	EndDraw();
}

void Shader::Pass::DrawIndexedInstanced(UINT indexCountPerInstance, UINT instanceCount, UINT startIndexLocation, INT baseVertexLocation, UINT startInstanceLocation)
{
	BeginDraw();
	{
		D3D::GetDC()->DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startIndexLocation);
	}
	EndDraw();
}
///////////////////////////////////////////////////////////////////////////////
void Shader::Pass::BeginDraw()
{
	//state ����� ȣ��
	//TODO : �ռ� ����� EndDraw���� �� ���°��� ���濩�θ� �����ϰ� �� ���� ��ο쿡�� ����� ���� ���氪�� ������?
	IPass->ComputeStateBlockMask(&StateBlockMask);
																	  
	D3D::GetDC()->IASetInputLayout(InputLayout);					  
	IPass->Apply(0, D3D::GetDC()); //pass�� ���Ե� state�� deivce�� ����
}

void Shader::Pass::EndDraw()
{
	if (StateBlockMask.RSRasterizerState == 1) //�����Ͷ����� ���°� ������
		D3D::GetDC()->RSSetState(StateBlock->RSRasterizerState);

	if (StateBlockMask.OMDepthStencilState == 1) //�𽺽��ٽ� ���°� ������
		D3D::GetDC()->OMSetDepthStencilState(StateBlock->OMDepthStencilState, StateBlock->OMStencilRef);

	if (StateBlockMask.OMBlendState == 1) //���� ���°� ������
		D3D::GetDC()->OMSetBlendState(StateBlock->OMBlendState, StateBlock->OMBlendFactor, StateBlock->OMSampleMask);
	//�׸� �� HS, DS, GS �ʱ�ȭ
	D3D::GetDC()->HSSetShader(NULL, NULL, 0);
	D3D::GetDC()->DSSetShader(NULL, NULL, 0);
	D3D::GetDC()->GSSetShader(NULL, NULL, 0);
}

void Shader::Pass::Dispatch(UINT x, UINT y, UINT z)
{
	IPass->Apply(0, D3D::GetDC());
	D3D::GetDC()->Dispatch(x, y, z);

	ID3D11ShaderResourceView* nullSRV[1] = { 0 };
	D3D::GetDC()->CSSetShaderResources(0, 1, nullSRV);

	ID3D11UnorderedAccessView* nullUAV[1] = { 0 };
	D3D::GetDC()->CSSetUnorderedAccessViews(0, 1, nullUAV, NULL);

	D3D::GetDC()->CSSetShader(NULL, NULL, 0); //CS ��Ȱ��
}
//TechniqueDraw////////////////////////////////////////////////////////////////
void Shader::Technique::Draw(UINT pass, UINT vertexCount, UINT startVertexLocation)
{
	Passes[pass].Draw(vertexCount, startVertexLocation);
}

void Shader::Technique::DrawIndexed(UINT pass, UINT indexCount, UINT startIndexLocation, INT baseVertexLocation)
{
	Passes[pass].DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
}

void Shader::Technique::DrawInstanced(UINT pass, UINT vertexCountPerInstance, UINT instanceCount, UINT startVertexLocation, UINT startInstanceLocation)
{
	Passes[pass].DrawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
}

void Shader::Technique::DrawIndexedInstanced(UINT pass, UINT indexCountPerInstance, UINT instanceCount, UINT startIndexLocation, INT baseVertexLocation, UINT startInstanceLocation)
{
	Passes[pass].DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
}

void Shader::Technique::Dispatch(UINT pass, UINT x, UINT y, UINT z)
{
	Passes[pass].Dispatch(x, y, z);
}
///////////////////////////////////////////////////////////////////////////////

//Shader Draw//////////////////////////////////////////////////////////////////
void Shader::Draw(UINT technique, UINT pass, UINT vertexCount, UINT startVertexLocation)
{
	techniques[technique].Passes[pass].Draw(vertexCount, startVertexLocation);
}

void Shader::DrawIndexed(UINT technique, UINT pass, UINT indexCount, UINT startIndexLocation, UINT baseVertexLocation)
{
	techniques[technique].Passes[pass].DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
}

void Shader::DrawInstanced(UINT technique, UINT pass, UINT vertexCountPerInstance, UINT instanceCount, UINT startVertexLocation, UINT startInstanceLocation)
{
	techniques[technique].Passes[pass].DrawInstanced(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
}

void Shader::DrawIndexedInstanced(UINT technique, UINT pass, UINT indexCountPerInstance, UINT instanceCount, UINT startIndexLocation, INT baseVertexLocation, UINT startInstanceLocation)
{
	techniques[technique].Passes[pass].DrawIndexedInstanced(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
}
void Shader::Dispatch(UINT technique, UINT pass, UINT x, UINT y, UINT z)
{
	techniques[technique].Passes[pass].Dispatch(x, y, z);
}
///////////////////////////////////////////////////////////////////////////////
//ID3DX11EffectVariable -> ��� Effect������ �⺻ Ŭ����
ID3DX11EffectVariable * Shader::Variable(string name)
{
	return effect->GetVariableByName(name.c_str());
}

ID3DX11EffectScalarVariable * Shader::AsScalar(string name)
{
	return effect->GetVariableByName(name.c_str())->AsScalar();
}

ID3DX11EffectVectorVariable * Shader::AsVector(string name)
{
	return effect->GetVariableByName(name.c_str())->AsVector();
}

ID3DX11EffectMatrixVariable * Shader::AsMatrix(string name)
{
	return effect->GetVariableByName(name.c_str())->AsMatrix();
}

ID3DX11EffectStringVariable * Shader::AsString(string name)
{
	return effect->GetVariableByName(name.c_str())->AsString();
}

ID3DX11EffectShaderResourceVariable * Shader::AsSRV(string name)
{
	return effect->GetVariableByName(name.c_str())->AsShaderResource();
}

ID3DX11EffectRenderTargetViewVariable * Shader::AsRTV(string name)
{
	return effect->GetVariableByName(name.c_str())->AsRenderTargetView();
}

ID3DX11EffectDepthStencilViewVariable * Shader::AsDSV(string name)
{
	return effect->GetVariableByName(name.c_str())->AsDepthStencilView();
}

ID3DX11EffectUnorderedAccessViewVariable * Shader::AsUAV(string name)
{
	return effect->GetVariableByName(name.c_str())->AsUnorderedAccessView();
}

ID3DX11EffectConstantBuffer * Shader::AsConstantBuffer(string name)
{
	return effect->GetConstantBufferByName(name.c_str());
}

ID3DX11EffectShaderVariable * Shader::AsShader(string name)
{
	return effect->GetVariableByName(name.c_str())->AsShader();
}

ID3DX11EffectBlendVariable * Shader::AsBlend(string name)
{
	return effect->GetVariableByName(name.c_str())->AsBlend();
}

ID3DX11EffectDepthStencilVariable * Shader::AsDepthStencil(string name)
{
	return effect->GetVariableByName(name.c_str())->AsDepthStencil();
}

ID3DX11EffectRasterizerVariable * Shader::AsRasterizer(string name)
{
	return effect->GetVariableByName(name.c_str())->AsRasterizer();
}

ID3DX11EffectSamplerVariable * Shader::AsSampler(string name)
{
	return effect->GetVariableByName(name.c_str())->AsSampler();
}

///////////////////////////////////////////////////////////////////////////////

unordered_map<wstring, Shaders::ShaderDesc> Shaders::shaders;

void Shaders::GetEffect(wstring fileName, ID3DBlob ** blob, ID3DX11Effect ** effect)
{
	bool isFind = false;

	//fx�� ������ ������ ������->effect�μ��� ����Ʈ ����
	if (shaders.count(fileName) < 1) //��Ұ� �������� ������(�����ϸ� 1��ȯ, �ƴϸ� 0��ȯ)
	{
		Pair p; //wstring, ShaderDesc

		//��ã���� ���
		ID3DBlob* error;
		// TODO : ���̴� ������ �ɼ� �÷��� - ������ ���̴� 4_0�� ������ | ��� ����� ��켱
		INT flag = D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY | D3D10_SHADER_PACK_MATRIX_ROW_MAJOR;
		//����Ʈ�ڵ�� ���̴� ������
		//TODO : D3DCompileFromFile(fileName.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, NULL, "fx_5_0", flag, NULL, &p.second.blob, &error); -> WindowsSDK
		//		 D3DX11CompileFromFile() -> DirectX SDK
		HRESULT hr = D3DCompileFromFile(fileName.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, NULL, "fx_5_0", flag, NULL, &p.second.blob, &error);
		if (FAILED(hr))
		{
			if (error != NULL) //�����޽����� �����Ѵٸ�ss
			{
				string str = (const char *)error->GetBufferPointer();
				MessageBoxA(NULL, str.c_str(), "Shader Error", MB_OK);
			}
			assert(false); //���α׷� �ߴ�
		}
		//����Ʈ ���� ����
		//TODO : 3��° ���� flagȮ��(���� D3DCompileFromFile�� flag���ڿ� ��ġ?)
		hr = D3DX11CreateEffectFromMemory(p.second.blob->GetBufferPointer(), p.second.blob->GetBufferSize(), 0, D3D::GetDevice(), &p.second.effect);
		assert(SUCCEEDED(hr));

		p.first = fileName;

		shaders.insert(p);

		*blob = p.second.blob;
		p.second.effect->CloneEffect(D3DX11_EFFECT_CLONE_FORCE_NONSINGLE, effect);
	}
	else //fx�� ������ ������ ����, ����Ʈ ����
	{
		ShaderDesc desc = shaders.at(fileName);

		*blob = desc.blob;
		desc.effect->CloneEffect(D3DX11_EFFECT_CLONE_FORCE_NONSINGLE, effect);
	}
}

void Shaders::Delete()
{
	for (Pair p : shaders)
	{
		SAFE_RELEASE(p.second.blob);
		SAFE_RELEASE(p.second.effect);
	}
}
