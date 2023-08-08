#include "Framework.h"
#include "Billboard.h"
#include "Boundings/Ray.h"
#include "Environment/Terrain.h"

Billboard::Billboard(Terrain* terrain)
	: terrain(terrain)
	, instanceCount(0), addCount(30)
	, useUndo(true), useRedo(false)
{
	shader = new Shader(Shaders + L"Billboard.fx");
	
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
	//TODO : 나중에 카피안하고 참조형으로 받게 수정필요(Mouse클래스 TerrainPicked함수도)
	//TODO : 나중에 브러쉬 적용되면 바뀐 지면 정점을 다시 복사해야함->그래서 참조형으로 받아야함
	{
		UINT tVertexCount = terrain->VertexCount();
		terrainPositions = new D3DXVECTOR3[tVertexCount];
		tVertices = new VertexTextureNormalTangent[tVertexCount];
		terrain->CopyVertices((void*)tVertices);
		
		for (int i = 0; i < tVertexCount; i++)
		{
			terrainPositions[i] = tVertices[i].Position;
			terrainY[(const int)terrainPositions[i].x][(const int)terrainPositions[i].z] = terrainPositions[i].y;
		}

		SAFE_DELETE_ARRAY(tVertices);
	}

	instanceVertex.Size = D3DXVECTOR2(2.0f, 2.0f);
	//instanceVertex.Size = D3DXVECTOR2(Math::Random(1.2f, 2.0f), Math::Random(1.8f, 2.0f));
	instanceVertex.Position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	//CreateVertexBuffer -> 인스턴싱이기 때문에 최초 한번만 생성 후 instance객체로 갯수를 늘림
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

Billboard::~Billboard()
{
	SAFE_DELETE(shader);
	
	SAFE_DELETE(texture);

	SAFE_DELETE_ARRAY(tVertices);
	SAFE_DELETE_ARRAY(instance);
}

void Billboard::Update()
{
	ImGui::InputInt("Billboard Count", &addCount, 1, 50); //한번에 추가시킬 빌보드 갯수 표시
	ImGui::InputInt("Current Billboard Count", &instanceCount); //총 빌보드 갯수 표시

	ImGui::Checkbox("Use Billboard Undo", &useUndo);
	ImGui::Checkbox("Use Billboard Redo", &useRedo);

	//Update Instance
	if (Mouse::Get()->Down(0))
	{
		UpdateInstanceData();

	}
	else if (Mouse::Get()->Down(1))
	{
		if (useUndo == true && undoCount.size() != 0)
		{
			redoCount.push(undoCount.top());

			instanceCount -= undoCount.top();
			bDatas.resize(instanceCount);

			undoCount.pop();
		}
		else if (useRedo == true && redoCount.size() != 0)
		{
			instanceCount += redoCount.top();
			bDatas.resize(instanceCount);
			redoCount.pop();
		}
	}
}

void Billboard::UpdateInstanceData()
{
	Vertex* instance = new Vertex[addCount]; //인스턴스 추가시키기 위한 임시 데이터

	instanceCount += addCount;

	//Undo용 카운트 push
	undoCount.push(addCount);

	//마우스 위치
	D3DXVECTOR3 pickedPos;
	terrain->TerrainPicked(&pickedPos, *ray);
	
	for (int i = 0; i < addCount; i++) //bDatas에 데이터 추가
	{
		float X = Math::Random(pickedPos.x - 10, pickedPos.x + 10);
		float Z = Math::Random(pickedPos.z - 10, pickedPos.z + 10);
		float Y = instanceVertex.Size.y * 0.5f + (terrainY[(const int)X][(const int)Z]);
		instance[i].Position = D3DXVECTOR3(X, Y, Z);
		
		bDatas.push_back(instance[i]);
	}

	//Create Instance Buffer
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(Vertex) * instanceCount; //instanceCount
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	
		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = &bDatas[0];
	
		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &vertexBuffer[1]);
		assert(SUCCEEDED(hr));
	}

	SAFE_DELETE_ARRAY(instance);
}

void Billboard::Render()
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

void Billboard::Undo()
{
	vector<Vertex>::iterator iter = bDatas.begin();
	
	int currentCount = instanceCount;
	for (iter = bDatas.end() - 1; currentCount >= (instanceCount - addCount) ; iter--)
	{
		bDatas.erase(iter);
		currentCount--;
	}
	
	instanceCount -= addCount;
	bDatas.resize(instanceCount);
}
