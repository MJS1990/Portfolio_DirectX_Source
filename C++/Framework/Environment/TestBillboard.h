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

	VertexSize vertices[50]; //�ν��Ͻ� ��ü �߽���

	//0->�����̵� instancePos. 1->�� �ν��Ͻ��� ����
	ID3D11Buffer* vertexBuffer;

	float terrainHeight;
	UINT terrainX, terrainZ;

	D3DXVECTOR3 pickedPos; //���콺 ���� ��ŷ ��ġ��
	class Ray* ray;

	VertexTextureNormalTangent* tempVertices;
	D3DXVECTOR3* tPosition; //���� ������ġ����

	Texture* texture;
};