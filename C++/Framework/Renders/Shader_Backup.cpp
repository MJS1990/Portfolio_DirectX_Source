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
	ID3DBlob* fxBlob; //컴파일시 각 파이프라인에 들어가기 전에 코드를 저장하는 지점

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
	//TODO : SAFE_RESLEASE(fxBlob) 주석
	//SAFE_RELEASE(fxBlob);
}

ID3D11InputLayout * Shader::CreateInputLayout(ID3DBlob * fxBlob, D3DX11_EFFECT_SHADER_DESC * effectVsDesc, vector<D3D11_SIGNATURE_PARAMETER_DESC>& params)
{
	//TODO : std::
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;

	for (D3D11_SIGNATURE_PARAMETER_DESC& paramDesc : params)
	{
		//InputAssmbler단계에서 단일 엘리먼트 Desc
		D3D11_INPUT_ELEMENT_DESC elementDesc;
		elementDesc.SemanticName = paramDesc.SemanticName;
		elementDesc.SemanticIndex = paramDesc.SemanticIndex;
		elementDesc.InputSlot = 0;
		elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT; //오프셋 자동
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
		transform(name.begin(), name.end(), name.begin(), toupper); //name 대문자 변경후 저장
		
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

	if (inputLayoutDesc.size() > 0) //버퍼가 존재하면 InputLayout생성
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
	//state 변경시 호출
	//TODO : 앞서 실행된 EndDraw에서 각 상태값의 변경여부를 저장하고 그 다음 드로우에서 저장된 상태 변경값을 가져옴?
	IPass->ComputeStateBlockMask(&StateBlockMask);
																	  
	D3D::GetDC()->IASetInputLayout(InputLayout);					  
	IPass->Apply(0, D3D::GetDC()); //pass에 포함된 state를 deivce로 설정
}

void Shader::Pass::EndDraw()
{
	if (StateBlockMask.RSRasterizerState == 1) //래스터라이저 상태값 보존시
		D3D::GetDC()->RSSetState(StateBlock->RSRasterizerState);

	if (StateBlockMask.OMDepthStencilState == 1) //댑스스텐실 상태값 보존시
		D3D::GetDC()->OMSetDepthStencilState(StateBlock->OMDepthStencilState, StateBlock->OMStencilRef);

	if (StateBlockMask.OMBlendState == 1) //블랜드 상태값 보존시
		D3D::GetDC()->OMSetBlendState(StateBlock->OMBlendState, StateBlock->OMBlendFactor, StateBlock->OMSampleMask);
	//그린 후 HS, DS, GS 초기화
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

	D3D::GetDC()->CSSetShader(NULL, NULL, 0); //CS 비활성
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
//ID3DX11EffectVariable -> 모든 Effect변수의 기본 클래스
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

	//fx가 없으면 설정후 컴파일->effect인수에 이펙트 복사
	if (shaders.count(fileName) < 1) //요소가 존재하지 않으면(존재하면 1반환, 아니면 0반환)
	{
		Pair p; //wstring, ShaderDesc

		//못찾았을 경우
		ID3DBlob* error;
		// TODO : 쉐이더 컴파일 옵션 플래그 - 구버전 쉐이더 4_0모델 컴파일 | 행렬 입출력 행우선
		INT flag = D3D10_SHADER_ENABLE_BACKWARDS_COMPATIBILITY | D3D10_SHADER_PACK_MATRIX_ROW_MAJOR;
		//바이트코드로 쉐이더 컴파일
		//TODO : D3DCompileFromFile(fileName.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, NULL, "fx_5_0", flag, NULL, &p.second.blob, &error); -> WindowsSDK
		//		 D3DX11CompileFromFile() -> DirectX SDK
		HRESULT hr = D3DCompileFromFile(fileName.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, NULL, "fx_5_0", flag, NULL, &p.second.blob, &error);
		if (FAILED(hr))
		{
			if (error != NULL) //에러메시지가 존재한다면ss
			{
				string str = (const char *)error->GetBufferPointer();
				MessageBoxA(NULL, str.c_str(), "Shader Error", MB_OK);
			}
			assert(false); //프로그램 중단
		}
		//이펙트 파일 생성
		//TODO : 3번째 인자 flag확인(위의 D3DCompileFromFile의 flag인자와 일치?)
		hr = D3DX11CreateEffectFromMemory(p.second.blob->GetBufferPointer(), p.second.blob->GetBufferSize(), 0, D3D::GetDevice(), &p.second.effect);
		assert(SUCCEEDED(hr));

		p.first = fileName;

		shaders.insert(p);

		*blob = p.second.blob;
		p.second.effect->CloneEffect(D3DX11_EFFECT_CLONE_FORCE_NONSINGLE, effect);
	}
	else //fx가 있으면 설정값 복사, 이펙트 복사
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
