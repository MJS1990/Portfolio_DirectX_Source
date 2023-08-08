#include "stdafx.h"
#include "DrawTerrain.h"
#include "TessTerrain/TessTerrain.h"
#include "TessTerrain/TessHeightMap.h"
#include "Environment/Sky.h"
//test
#include "Environment/Terrain.h"
#include "Environment/QuadTree.h"

DrawTerrain::DrawTerrain()
{
}

DrawTerrain::~DrawTerrain()
{
}

void DrawTerrain::Initialize()
{
	//CreateTerrain
	{
		TessTerrain::InitDesc desc =
		{
			Assets + L"/TerrainTexture/rock_boulder_cracked_disp_4k.png", //HeightMap
			//Contents + L"HeightMaps/Map_4.raw", //HeightMap
			50.0f, //HeightScale
			2049, 2049, //HeigtMapWidth, HeightMapHeight
			Textures + L"ground5_Diffuse.dds",
			Textures + L"ground5_Normal.tga",
			0.5f, //CellSpacing
		};

		terrain = new TessTerrain(desc);
		terrain->Initialize();
	}

	sSky = new Sky();
	sSky->Initialize();

	testTerrain = new Terrain();
	//펄린 노이즈 지형
	quadTree = new QuadTree(testTerrain, Context::Get()->GetFrustum());
	quadTree->Initialize();
}

void DrawTerrain::Ready()
{
	//terrainMaterial = new Material(Shaders + L"060_Terrain.fx");
	terrainMaterial = new Material(Shaders + L"TessTerrain.fx");
	terrain->Ready(terrainMaterial);
	
	sSky->Ready();
}

void DrawTerrain::Destroy()
{
	SAFE_DELETE(terrainMaterial);
	SAFE_DELETE(terrain);

	sSky->Destroy();
	SAFE_DELETE(sSky);

	SAFE_DELETE(testTerrain);
	SAFE_DELETE(quadTree);
}

void DrawTerrain::Update()
{
	sSky->Update();

	quadTree->Update();
}

void DrawTerrain::PreRender()
{
	sSky->PreRender();
}

void DrawTerrain::Render()
{
	//terrain->Render();
	
	sSky->Render();

	quadTree->Render();
}

void DrawTerrain::PostRender()
{
	sSky->PostRender();
}

void DrawTerrain::ResizeScreen()
{
}
