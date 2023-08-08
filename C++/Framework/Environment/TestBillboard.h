#pragma once

class TestBillboard
{
public:
	TestBillboard(class Terrain* terrain);
	~TestBillboard();

	void Update();
	void Render();

private:
	Shader* shader;

	class Terrain* terrain;

	VertexSize vertices[50]; //인스턴스 객체 중심점

	//0->정점이될 instancePos. 1->각 인스턴스별 월드
	ID3D11Buffer* vertexBuffer;

	float terrainHeight;
	UINT terrainX, terrainZ;

	D3DXVECTOR3 pickedPos; //마우스 지면 피킹 위치값
	class Ray* ray;

	VertexTextureNormalTangent* tempVertices;
	D3DXVECTOR3* tPosition; //지형 정점위치값들

	Texture* texture;
};