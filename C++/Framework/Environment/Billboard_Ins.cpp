#include "Framework.h"
#include "Billboard_Ins.h"
#include "Boundings/Ray.h"
#include "Environment/Terrain.h"

Billboard_Ins::Billboard_Ins(Terrain* terrain)
	: terrain(terrain)
	, instanceCount(0), addCount(30)
	, useUndo(true)
{
	shader = new Shader(Shaders + L"Billboard_Ins.fx");
	
	ray = new Ray();
	//Initialize Texture
	{
		texture = new Texture(Textures + L"Billboard_Inss/Billboard_Ins_01.dds");
		shader->AsSRV("Billboard_InsTex")->SetResource(texture->SRV());
	}

	//��ŷ�� �ʿ��� ���� ���̰�
	terrainX = this->terrain->GetWidth();
	terrainZ = this->terrain->GetHeight();

	//CopyTerrainVertices
	{
		UINT tVertexCount = terrain->VertexCount();
		terrainPositions = new D3DXVECTOR3[tVertexCount];
		tVertices = new VertexTextureNormalTangent[tVertexCount];
		terrain->CopyVertices((void*)tVertices);
		
		for (UINT i = 0; i < tVertexCount; i++)
		{
			terrainPositions[i] = tVertices[i].Position;
			terrainY[(const int)terrainPositions[i].x][(const int)terrainPositions[i].z] = terrainPositions[i].y;
		}

		SAFE_DELETE_ARRAY(tVertices);
	}

	instanceVertex.Size = D3DXVECTOR2(2.0f, 2.0f);
	instanceVertex.Position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	//CreateVertexBuffer -> �ν��Ͻ��̱� ������ ���� �ѹ��� ���� �� instance��ü�� ������ �ø�
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(VertexSize);
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	
		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = &instanceVertex;
	
		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &vertexBuffer[0]);
		assert(SUCCEEDED(hr));
	}
}

Billboard_Ins::~Billboard_Ins()
{
	SAFE_DELETE(shader);
	
	SAFE_DELETE(texture);

	SAFE_DELETE_ARRAY(tVertices);
	SAFE_DELETE_ARRAY(instance);
}

void Billboard_Ins::Update()
{
	ImGui::InputInt("Add Billboard_Ins Count", &addCount, 1, 50); //�ѹ��� �߰���ų ������ ���� ǥ��
	ImGui::InputInt("Current Billboard_Ins Count", &instanceCount); //�� ������ ���� ǥ��

	ImGui::Checkbox("Use Billboard_Ins Undo", &useUndo);

	//Update Instance
	if (Mouse::Get()->Down(0))
	{
		UpdateInstanceData();
	}
}

void Billboard_Ins::UpdateInstanceData()
{
	Vertex* instance = new Vertex[addCount]; //�ν��Ͻ� �߰���Ű�� ���� �ӽ� ������

	instanceCount += addCount;

	//���콺 ��ġ
	D3DXVECTOR3 pickedPos;
	terrain->TerrainPicked(&pickedPos, *ray);

	for (int i = 0; i < addCount; i++) //bDatas�� ������ �߰�
	{
		float X = Math::Random(pickedPos.x - 10, pickedPos.x + 10);
		float Z = Math::Random(pickedPos.z - 10, pickedPos.z + 10);
		float Y = instanceVertex.Size.y * 0.5f + (terrainY[(const int)X][(const int)Z]);
		instance[i].Position = D3DXVECTOR3(X, Y, Z);

		bDatas.push_back(instance[i]);
	}

	SAFE_DELETE_ARRAY(instance);
	//Create Instance Buffer
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(Vertex) * instanceCount;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		
		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = &bDatas[0];
	
		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &vertexBuffer[1]);
		assert(SUCCEEDED(hr));
	}
}

void Billboard_Ins::Render()
{
	if (instanceCount == 0) return;

	UINT stride[2];
	stride[0] = sizeof(VertexSize);
	stride[1] = sizeof(Vertex);
	UINT offset[2] = { 0, 0 };

	D3D::GetDC()->IASetVertexBuffers(0, 2, vertexBuffer, stride, offset);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	shader->DrawInstanced(0, 0, 1, instanceCount);
}

void Billboard_Ins::Undo()
{
	if (Mouse::Get()->Down(1))
	{
		if (useUndo == true && instanceCount > 0)
		{
			instanceCount -= addCount;
			bDatas.resize(instanceCount);
		}
	}
}
