#include "Framework.h"
#include "TestBillboard.h"
#include "Boundings/Ray.h"
#include "Environment/Terrain.h"

TestBillboard::TestBillboard(Terrain* terrain)
	: terrain(terrain)
{
	shader = new Shader(Shaders + L"TestBillboard.fx");

	ray = new Ray();
	//Initialize Texture
	{
		texture = new Texture(Textures + L"Billboards/Billboard_01.dds");
		shader->AsSRV("BillboardTex")->SetResource(texture->SRV());
	}

	//피킹에 필요한 지면 넓이값
	terrainX = this->terrain->GetWidth();
	terrainZ = this->terrain->GetHeight();

	//CopyTerrainVertices
	{
		UINT tVertexCount = terrain->VertexCount();
		tempVertices = new VertexTextureNormalTangent[tVertexCount];
		tPosition = new D3DXVECTOR3[tVertexCount];

		for (UINT i = 0; i < tVertexCount; i++)
		{
			tPosition[i] = tempVertices[i].Position;
		}
	}

	SAFE_DELETE_ARRAY(tempVertices);


	for (int i = 0; i < 50; i++)
	{
		vertices[i].Size = D3DXVECTOR2(10.0f, 10.0f);
		vertices[i].Position = D3DXVECTOR3(static_cast<float>(Math::Random(0, 255)), vertices[i].Size.y * 0.5f, static_cast<float>(Math::Random(0, 255)));
	}

	//CreateVertexBuffer
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(VertexSize) * 50;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = vertices;

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &vertexBuffer);
		assert(SUCCEEDED(hr));
	}
}

TestBillboard::~TestBillboard()
{
	SAFE_DELETE(shader);
	
	SAFE_DELETE(texture);

	SAFE_DELETE_ARRAY(tPosition);
}

void TestBillboard::Update()
{
}

void TestBillboard::Render()
{
	UINT stride;
	stride = sizeof(VertexSize);
	UINT offset = 0;

	D3D::GetDC()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	shader->Draw(0, 0, 50);
}