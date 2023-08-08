#pragma once

//지형용 컨트롤포인트 구조체?
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
	static const UINT CellPerPatch; //셀 하나당 패치갯수

private:
	class TerrainRender* tRender;
	class TessHeightMap* heightMap;


public:
	struct InitDesc //초기데이터값
	{
		wstring HeightMap;
		float HeightScale;
		float HeightMapWidth;
		float HeightMapHeight;

		wstring MapFile;
		float CellSpacing; //셀 하나당 너비?

		Material* material;
	};

private:
	InitDesc desc;

public:
	InitDesc& Desc() { return desc; }
};

