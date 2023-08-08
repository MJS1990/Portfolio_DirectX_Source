#pragma once

//������ ��Ʈ������Ʈ ����ü?
struct TerrainCP
{
	D3DXVECTOR3 Position;
	//float Padding1;
	D3DXVECTOR2 Uv;
	D3DXVECTOR2 BoundsY;

	D3DXVECTOR3 Normal;
	//float Padding2;
	D3DXVECTOR3 Tangent;

	TerrainCP() {}
	TerrainCP(D3DXVECTOR3 position, D3DXVECTOR2 uv, D3DXVECTOR2 boundsY, D3DXVECTOR3 normal, D3DXVECTOR3 tangent)
	{
		Position = position;
		Uv = uv;
		BoundsY = boundsY;

		Normal = normal;
		Tangent = tangent;
	}
};

class TessTerrain
{
public:
	struct InitDesc;

public:
	TessTerrain(InitDesc& desc);
	~TessTerrain();

	void Initialize();
	void Ready(Material* material);

	void Update();
	void Render();

	void Data(UINT row, UINT col, float data);
	float Data(UINT row, UINT col);

	float Width();
	float Depth();
	float GetHeight(float x, float z);

	class TessHeightMap* GetHeightMap() { return heightMap; }

public:
	static const UINT CellPerPatch; //�� �ϳ��� ��ġ����

private:
	class TerrainRender* tRender;
	class TessHeightMap* heightMap;


public:
	struct InitDesc //�ʱⵥ���Ͱ�
	{
		wstring HeightMap;
		float HeightScale;
		float HeightMapWidth;
		float HeightMapHeight;

		wstring MapFile;
		float CellSpacing; //�� �ϳ��� �ʺ�?

		Material* material;
	};

private:
	InitDesc desc;

public:
	InitDesc& Desc() { return desc; }
};

