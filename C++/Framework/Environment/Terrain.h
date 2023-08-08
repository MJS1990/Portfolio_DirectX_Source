#pragma once

class Terrain
{
public:
	Terrain();
	~Terrain();

	//for QuadTree
	UINT VertexCount() { return vertexCount; }
	void CopyVertices(void* vertices);
	void AlignVertexData();
	
	UINT GetWidth() { return width; }
	UINT GetHeight() { return height; }
	D3DXMATRIX GetWorld() { return world; }

	bool TerrainPicked(OUT D3DXVECTOR3 * out, Ray & ray);

private:
	void ReadHeightMap(wstring file);
	void CalcNormalVector();
	void CalcTangentVector();

private:
	VertexTextureNormalTangent* vertices;
	UINT* indices;
	UINT vertexCount, indexCount;
	
	UINT width, height;

	//Drag ��������
	vector<UINT> dragIndices;
	float addHeight;
	//

private: //���� ��ŷ�� ����
	D3DXMATRIX world;
	
	D3DXVECTOR3 start;
	D3DXVECTOR3 direction;

	D3DXMATRIX view;
	D3DXMATRIX projection;
	//	

};