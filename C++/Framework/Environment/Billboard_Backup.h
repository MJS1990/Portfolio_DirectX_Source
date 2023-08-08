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

	VertexSize instanceVertex; //�ν��Ͻ� ��ü �߽���

	//0->�����̵� instancePos. 1->�� �ν��Ͻ��� ����
	ID3D11Buffer* vertexBuffer[2];
	//TextureArray* textureArray; //TODO : �׽�Ʈ �� ���

	float terrainHeight;
	float terrainX, terrainZ;

	D3DXVECTOR3 pickedPos; //���콺 ���� ��ŷ ��ġ��
	class Ray* ray;

	D3DXVECTOR3* terrainPositions;
	VertexTextureNormalTangent* tVertices; //���� ������


private:
	int instanceCount; //������ ��ü���� �� ����
	int addCount; //���콺�� �ѹ��� �߰���ų �������� ����
	D3DXMATRIX worlds[30];
	Texture* texture; //TODO : �ϴ� 1���� �׽�Ʈ, ���� �ؽ��� �迭���

	ID3D11Buffer* worldBuffer;

	Vertex* instance;

	vector<Vertex> bDatas;

	float terrainY[256][256];

	//Undo�� �ν��Ͻ����� ����
	stack<int> undoCount;
	//Redo�� �ӽ� �ν��Ͻ����� ���� ����
	//stack<Vertex> redoCount;
	stack<int> redoCount;

	bool useUndo;
	bool useRedo;
};