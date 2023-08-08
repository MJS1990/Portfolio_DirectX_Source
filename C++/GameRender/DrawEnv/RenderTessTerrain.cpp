#include "stdafx.h"
#include "RenderTessTerrain.h"
#include "TessTerrain/TessTerrain.h"
#include "TessTerrain/TessHeightMap.h"

void RenderTessTerrain::Initialize()
{
	//CreateTerrain
	{
		TessTerrain::InitDesc desc =
		{
			//Contents + L"HeightMaps/HeightMap256.dds", //HeightMap
			Contents + L"HeightMaps/Map_4.raw", //HeightMap
			50.0f, //HeightScale
			2049, 2049, //HeigtMapWidth, HeightMapHeight
			Textures + L"ground5_Diffuse.dds",
			Textures + L"ground5_Normal.tga",
			0.5f, //CellSpacing
		};

		terrain = new TessTerrain(desc);
		terrain->Initialize();
	}
}

void RenderTessTerrain::Ready()
{
	//terrainMaterial = new Material(Shaders + L"060_Terrain.fx");
	terrainMaterial = new Material(Shaders + L"TessTerrain.fx");
	terrain->Ready(terrainMaterial);
}

void RenderTessTerrain::Destroy()
{
	SAFE_DELETE(terrainMaterial);
	SAFE_DELETE(terrain);
}

void RenderTessTerrain::Update()
{
}

void RenderTessTerrain::PreRender()
{
}

void RenderTessTerrain::Render()
{
	terrain->Render();
}

void RenderTessTerrain::PostRender()
{
}

void RenderTessTerrain::ResizeScreen()
{
}
