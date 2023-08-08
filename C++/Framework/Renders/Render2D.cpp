#include "Framework.h"
#include "Render2D.h"

Render2D::Render2D(wstring shaderFile)
	: position(0.0f, 0.0f), scale(1.0f, 1.0f)
{
	String::Replace(&shaderFile, Shaders, L""); //쉐이더 경로 생성

	wstring file = L"";
	if (shaderFile.length() > 1) //생성자로 들어온 shaderFile이 존재하면
		file = Shaders + shaderFile;
	else
		file = Shaders + L"Render2D.fx"; //존재하지 않으면 디폴트 쉐이더 Render2D 세팅

	shader = new Shader(file, true); //true라서 Context멤버 바인드 안함

	//Create OrthoMatrix -> 직교투영
	{
		D3DDesc desc;
		D3D::GetDesc(&desc);
		//OrthoOffCenterLH -> 중심점 설정 가능
		D3DXMatrixOrthoOffCenterLH(&orthographic, 0, desc.Width, 0, desc.Height, -1, 1);
	}

	//Create VertexBuffer
	{
		VertexTexture* vertices = new VertexTexture[6];
		vertices[0].Position = D3DXVECTOR3(-0.5f, -0.5f, 0.0f);
		vertices[1].Position = D3DXVECTOR3(-0.5f,  0.5f, 0.0f);
		vertices[2].Position = D3DXVECTOR3( 0.5f, -0.5f, 0.0f);
		vertices[3].Position = D3DXVECTOR3( 0.5f, -0.5f, 0.0f);
		vertices[4].Position = D3DXVECTOR3(-0.5f,  0.5f, 0.0f);
		vertices[5].Position = D3DXVECTOR3( 0.5f,  0.5f, 0.0f);

		vertices[0].Uv = D3DXVECTOR2(0, 1);
		vertices[1].Uv = D3DXVECTOR2(0, 0); 
		vertices[2].Uv = D3DXVECTOR2(1, 1);
		vertices[3].Uv = D3DXVECTOR2(1, 1);
		vertices[4].Uv = D3DXVECTOR2(0, 0);
		vertices[5].Uv = D3DXVECTOR2(1, 0);

		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(VertexTexture) * 6;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = vertices;

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &vertexBuffer);
		assert(SUCCEEDED(hr));

		SAFE_DELETE_ARRAY(vertices);
	}

	//카메라 행렬 세팅(고정)
	D3DXMatrixLookAtLH(&view, &D3DXVECTOR3(0, 0, -1), &D3DXVECTOR3(0, 0, 0), &D3DXVECTOR3(0, 1, 0));

	viewVariable = shader->AsMatrix("View");
	projectionVariable = shader->AsMatrix("Projection");
	mapVariable = shader->AsSRV("Map");

	viewVariable->SetMatrix(view);
	projectionVariable->SetMatrix(orthographic);

	UpdateWorld();
}

Render2D::~Render2D()
{
	SAFE_DELETE(shader);

	SAFE_RELEASE(vertexBuffer);
}

void Render2D::Position(float x, float y)
{
	Position(D3DXVECTOR2(x, y));
}

void Render2D::Position(D3DXVECTOR2 vec)
{
	position = vec;

	UpdateWorld();
}

void Render2D::Scale(float x, float y)
{
	Scale(D3DXVECTOR2(x, y));
}

void Render2D::Scale(D3DXVECTOR2 vec)
{
	scale = vec;

	UpdateWorld();
}

void Render2D::Update()
{
	//UpdateWorld(); //TODO : 추가
}

void Render2D::Render()
{
	UINT stride = sizeof(VertexTexture);
	UINT offset = 0;

	D3D::GetDC()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	shader->Draw(0, 0, 6);
}

void Render2D::SRV(ID3D11ShaderResourceView * srv)
{
	mapVariable->SetResource(srv);
}

void Render2D::UpdateWorld()
{
	D3DXMATRIX W, S, T;
	
	D3DXMatrixScaling(&S, scale.x, scale.y, 1);
	D3DXMatrixTranslation(&T, position.x + scale.x * 0.5f, position.y + scale.y * 0.5f, 0);

	W = S * T;
	shader->AsMatrix("World")->SetMatrix(W);
}
