#pragma once

#include "../Pattern/Command.h"

class Billboard
{
public:
	Billboard(class Terrain* terrain);
	~Billboard();

	void Update();
	void Render();

	void Undo();
	
	void UpdateInstanceData();

private:
	bool useBillboard;

	Shader* shader;

	class Terrain* terrain;

	//VertexSize instanceVertex; //인스턴스 객체 중심점
	VertexInstancing instanceVertex;

	//0->정점이될 instancePos. 1->각 인스턴스별 월드
	ID3D11Buffer* vertexBuffer[2];


	float terrainHeight;
	UINT terrainX, terrainZ;

	D3DXVECTOR3 pickedPos; //마우스 지면 피킹 위치값
	class Ray* ray;

	D3DXVECTOR3* terrainPositions;
	VertexTextureNormalTangent* tVertices; //지형 정점들


private:
	int instanceCount; //빌보드 객체들의 총 갯수
	int addCount; //마우스로 한번에 추가시킬 빌보드의 갯수
	D3DXMATRIX worlds[30];
	Texture* texture; 

	ID3D11Buffer* worldBuffer;

	Vertex* instance;

	vector<Vertex> bDatas;

	float terrainY[256][256];
	
	bool useUndo;

};