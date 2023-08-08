#pragma once

//QuadTree -> ���� �ϳ��� �ؽ��ĸ� ������ ������ Ŭ����
class QuadTree
{
public:
	static const UINT DivideCount; //32x32

private:
	struct NodeType;

public:
	QuadTree(class Terrain* terrain, class Brush* brush, class Frustum* frustum = NULL);
	~QuadTree();

	void Initialize();
	void Ready();
	void Update();
	void Render();
	
private:
	void RenderNode(NodeType* node);
	void DeleteNode(NodeType* node);

	//�ڽ� ��� ������ �Լ�
	void CalcMeshDimensions(UINT vertexCount, float& centerX, float& centerZ, float& meshWidth);
	void CreateTreeNode(NodeType* node, float positionX, float positionZ, float width);
	//�ڽĳ�尡 �����ϴ� �ﰢ�� ���� 
	UINT ContainTriangleCount(float positionX, float positionZ, float width);
	bool IsTriangleContained(UINT index, float positionX, float positionZ, float width);

public:
	Shader* GetShader() { return material->GetShader(); }

private:
	Terrain* terrain;
	Brush* brush;
	VertexTextureNormalTangent* vertices;
	UINT vertexCount; //���� ���� �� ����
	UINT triangleCount, drawCount;

	NodeType* parent;

	class Frustum* frustum;

	Material* material;

	D3DXMATRIX world;

	Perspective* perspectiveForCulling;
	float centerX, centerZ, width;

private:
	//����ü�� �귯���� ����� ������ ��ġ���� ������ ���� �����ѵ�
	//RenderNode�Լ����� ����ü�� VertexBuffer ���ε� ����
	//VS�� CS���� ����� ���ο� �������� VertexBuffer�� �����ϰ�
	//VertexBuffer���ε��ؼ� ����� ������ ����
	struct NodeType //������ ��帶�� material�� �־�� ��
	{
		//��� ����
		float X, Z, Width;
		int TriangleCount;
		ID3D11Buffer* VertexBuffer, *IndexBuffer; //���� �������� ���۵�
		//�ڽĳ�� 4��
		NodeType* Childs[4];
		//TODO : �߰� - ��� �� �ϳ��� �ؽ��� ������
		Texture* nodeTexture;
		//���̰� ����� ���� ����
		vector<D3DXVECTOR3> Vertices;
		//list<D3DXVECTOR3> Vertices;
	};

private: //CS�� ������
	//CSData�� D3DXVECTOR3�� �ְų� ���ۿ� ����ü���� D3DXVECTOR3�ְ�
	//���ۿ� �ٷ� vector�� ������ ����
	struct CSData
	{
		vector<D3DXVECTOR3> ResultVertices;
		//D3DXVECTOR3 ResultVertices;
	};
	CSData* csData;

	ID3D11Texture2D* csTexture; //CS����� ����� �ؽ���

	ID3D11ShaderResourceView* csSrv;
	//���ϵǾ���� �������� ������ �� 
	//cpp���� �����͸� �ޱ����� ���۴� ���� �����ؾ��Ѵ�
	//uav�����͸� ���̴����� �ٷ� �޴´ٸ� ���۴� �ʿ����
	ID3D11UnorderedAccessView* csUav;

	ID3D11Buffer* outputBuffer;
	ID3D11Buffer* outputDebugBuffer;

	void InitCSDatas(VertexTextureNormalTangent* terrainData);
	//void InitCSDatas(NodeType* node); //CS���ε�
	//void CopyCSOutput(NodeType* node); //CS��� ���� ����
	////TODO : Test�� ��� ���� �Ǻ��ϰ� ����ġ���� �ϴ� �Լ�
	//void DispatchVertices(NodeType* node);

	Texture* csHeightTex;
};