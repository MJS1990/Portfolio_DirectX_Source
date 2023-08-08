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

	//�ڽĳ�� ������ �Լ�
	void CalcMeshDimensions(UINT vertexCount, float& centerX, float& centerZ, float& meshWidth);
	void CreateTreeNode(NodeType* node, float positionX, float positionZ, float width);
	//�ڽĳ�尡 �����ϴ� �ﰢ�� ����
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

private: //�ø��� ������
	UINT triangleCount, drawCount;
	
	NodeType* parent;

	class Frustum* frustum;

	Perspective* perspectiveForCulling;

private:
	struct VsBufferData
	{
		float WaveFrequency = 0.05f; //���� �󵵰�
		float WaveAmplitude = 1.0f; //���� ������

		D3DXVECTOR2 TextureScale = D3DXVECTOR2(14, 14);

		D3DXVECTOR2 BumpSpeed = D3DXVECTOR2(0.0f, 0.05f); //���� �ӵ�
		float BumpHeight = 0.6f; //���� ����

		float RunningTime = 0.0f; //�ð��帧
	};

	VsBufferData* vsBufferData;
	CBuffer* vsBuffer;

	struct PsBufferData
	{
		D3DXCOLOR ShallowColor = D3DXCOLOR(0.0f, 0.4f, 0.1f, 1.0f); //������
		D3DXCOLOR DeepColor = D3DXCOLOR(0.2f, 0.5f, 0.95f, 1.0f); //������
		
		float Bias = 0.8f; //����
		float Power = 0.5f; //����
		float Amount = 0.5f; //������
		float ShoreBlend = 35.0f; //���İ� ��ȭ����
	 
		D3DXVECTOR2 OceanSize; //���ؽ��� ��üũ��
		float HeightRatio = 7.5f; //���� ������

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