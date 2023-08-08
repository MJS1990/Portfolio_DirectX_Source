#pragma once

//QuadTree -> 노드당 하나의 텍스쳐를 가지게 수정한 클래스
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

	//자식 노드 나누는 함수
	void CalcMeshDimensions(UINT vertexCount, float& centerX, float& centerZ, float& meshWidth);
	void CreateTreeNode(NodeType* node, float positionX, float positionZ, float width);
	//자식노드가 포함하는 삼각형 갯수 
	UINT ContainTriangleCount(float positionX, float positionZ, float width);
	bool IsTriangleContained(UINT index, float positionX, float positionZ, float width);

public:
	Shader* GetShader() { return material->GetShader(); }

private:
	Terrain* terrain;
	Brush* brush;
	VertexTextureNormalTangent* vertices;
	UINT vertexCount; //지면 정점 총 갯수
	UINT triangleCount, drawCount;

	NodeType* parent;

	class Frustum* frustum;

	Material* material;

	D3DXMATRIX world;

	Perspective* perspectiveForCulling;
	float centerX, centerZ, width;

private:
	//구조체에 브러쉬로 변경된 정점의 위치값을 저장할 변수 선언한뒤
	//RenderNode함수에서 구조체의 VertexBuffer 바인드 전에
	//VS나 CS에서 계산한 새로운 정점값을 VertexBuffer에 대입하고
	//VertexBuffer바인드해서 변경된 정점을 저장
	struct NodeType //원래는 노드마다 material이 있어야 함
	{
		//노드 정보
		float X, Z, Width;
		int TriangleCount;
		ID3D11Buffer* VertexBuffer, *IndexBuffer; //실제 랜더링될 버퍼들
		//자식노드 4개
		NodeType* Childs[4];
		//TODO : 추가 - 노드 당 하나의 텍스쳐 랜더링
		Texture* nodeTexture;
		//높이값 계산결과 담을 변수
		vector<D3DXVECTOR3> Vertices;
		//list<D3DXVECTOR3> Vertices;
	};

private: //CS용 데이터
	//CSData에 D3DXVECTOR3만 넣거나 버퍼에 구조체없이 D3DXVECTOR3넣고
	//버퍼에 바로 vector가 들어가도록 수정
	struct CSData
	{
		vector<D3DXVECTOR3> ResultVertices;
		//D3DXVECTOR3 ResultVertices;
	};
	CSData* csData;

	ID3D11Texture2D* csTexture; //CS결과물 저장용 텍스쳐

	ID3D11ShaderResourceView* csSrv;
	//리턴되어오는 데이터의 형태일 뿐 
	//cpp에서 데이터를 받기위한 버퍼는 따로 정의해야한다
	//uav데이터를 쉐이더에서 바로 받는다면 버퍼는 필요없음
	ID3D11UnorderedAccessView* csUav;

	ID3D11Buffer* outputBuffer;
	ID3D11Buffer* outputDebugBuffer;

	void InitCSDatas(VertexTextureNormalTangent* terrainData);
	//void InitCSDatas(NodeType* node); //CS바인드
	//void CopyCSOutput(NodeType* node); //CS결과 노드로 복사
	////TODO : Test용 노드 정점 판별하고 디스패치까지 하는 함수
	//void DispatchVertices(NodeType* node);

	Texture* csHeightTex;
};