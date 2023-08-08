#include "Framework.h"
#include "TessTerrain.h"
#include "TessHeightMap.h"
#include "TerrainRender.h"

const UINT TessTerrain::CellPerPatch = 64; //64개를 하나의 패치로 묶음(패치 32개)

TessTerrain::TessTerrain(InitDesc & desc)
	: desc(desc)
{

}

TessTerrain::~TessTerrain()
{
	SAFE_DELETE(heightMap);
	SAFE_DELETE(tRender);
}

void TessTerrain::Initialize()
{
	tRender = new TerrainRender(this);

	heightMap = new TessHeightMap((UINT)desc.HeightMapWidth, (UINT)desc.HeightMapHeight, desc.HeightScale);
	heightMap->Load(desc.HeightMap);

	tRender->Initialize();
}

void TessTerrain::Ready(Material * material)
{
	desc.material = material;

	tRender->Ready(material);
}

void TessTerrain::Update()
{
}

void TessTerrain::Render()
{
	tRender->Render();
}

void TessTerrain::Data(UINT row, UINT col, float data)
{
	heightMap->Data(row, col, data);
}

float TessTerrain::Data(UINT row, UINT col)
{
	return heightMap->Data(row, col);
}

float TessTerrain::Width()
{
	return (desc.HeightMapWidth - 1) * desc.CellSpacing;
}

float TessTerrain::Depth()
{
	return (desc.HeightMapHeight - 1) * desc.CellSpacing;
}

float TessTerrain::GetHeight(float x, float z)
{
	float c = (x + 0.5f * Width()) / desc.CellSpacing;
	float d = (z - 0.5f * Depth()) / -desc.CellSpacing;
	int row = (int)floorf(d);
	int col = (int)floorf(c);

	float h00 = heightMap->Data(row, col);
	float h01 = heightMap->Data(row, col + 1);
	float h10 = heightMap->Data(row + 1, col); 
	float h11 = heightMap->Data(row + 1, col + 1);

	float s = c - col;
	float t = d - row;

	float uy, vy;
	if (s + t <= 1.0f)
	{
		uy = h01 - h00;
		vy = h01 - h11;

		return h00 + (1.0f - s) * uy + (1.0f - t) * vy;
	}

	uy = h10 - h11;
	vy = h01 - h11;

	return h11 + (1.0f - s) * uy + (1.0f - t) * vy;
}
