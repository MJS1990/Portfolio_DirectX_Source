#pragma once

class Water
{
public:
	static const UINT DivideCount;

private:
	struct NodeType;

public:
	Water(UINT width, UINT height, float thick, Texture* heightMap = NULL, class Frustum* frustum = NULL);
	~Water();

	void Initialize();
	void Ready();
	
	void Update();
	void Render();
	
private:
	void RenderNode(NodeType* node);
	void DeleteNode(NodeType* node);

	//자식노드 나누는 함수
	void CalcMeshDimensions(UINT vertexCount, float& centerX, float& centerZ, float& meshWidth);
	void CreateTreeNode(NodeType* node, float positionX, float positionZ, float width);
	//자식노드가 포함하는 삼각형 갯수
	UINT ContainTriangleCount(float positionX, float positionZ, float width);
	bool IsTriangleContained(UINT index, float positionX, float positionZ, float width);

	void AlignVertexData();

private:
	Shader* shader;

	Material* material;
	ID3D11Buffer* vertexBuffer, *indexBuffer;
	VertexTexture* vertices;
	UINT* indices;
	UINT width, height;
	UINT vertexCount, indexCount;

	float thick;

	Texture* heightMap;

private: //컬링용 변수들
	UINT triangleCount, drawCount;
	
	NodeType* parent;

	class Frustum* frustum;

	Perspective* perspectiveForCulling;

private:
	struct VsBufferData
	{
		float WaveFrequency = 0.05f; //물결 빈도값
		float WaveAmplitude = 1.0f; //물결 진폭값

		D3DXVECTOR2 TextureScale = D3DXVECTOR2(14, 14);

		D3DXVECTOR2 BumpSpeed = D3DXVECTOR2(0.0f, 0.05f); //물결 속도
		float BumpHeight = 0.6f; //물결 높이

		float RunningTime = 0.0f; //시간흐름
	};

	VsBufferData* vsBufferData;
	CBuffer* vsBuffer;

	struct PsBufferData
	{
		D3DXCOLOR ShallowColor = D3DXCOLOR(0.0f, 0.4f, 0.1f, 1.0f); //얕은색
		D3DXCOLOR DeepColor = D3DXCOLOR(0.2f, 0.5f, 0.95f, 1.0f); //깊은색
		
		float Bias = 0.8f; //편향
		float Power = 0.5f; //강도
		float Amount = 0.5f; //보정값
		float ShoreBlend = 35.0f; //알파값 변화정도
	 
		D3DXVECTOR2 OceanSize; //버텍스의 전체크기
		float HeightRatio = 7.5f; //높이 보정값

		float Padding;
	};

	PsBufferData* psBufferData;
	CBuffer* psBuffer;


private:
	D3DXVECTOR2 oceanSize;
	float runningTime;

	Texture* normalTexture;

private:
	struct NodeType
	{
		float X, Z, Width;
		int TriangleCount;
		ID3D11Buffer* VertexBuffer, *IndexBuffer;
		
		NodeType* Childs[4];
	};
};