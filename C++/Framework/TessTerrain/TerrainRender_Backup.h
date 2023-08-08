#pragma once

#include "TessTerrain.h"

class TerrainRender
{
public:
	TerrainRender(class TessTerrain* terrain);
	~TerrainRender();

	void Initialize();
	void Ready(Material* material);

	void Render();

public:
	static const float MaxDistance;
	static const float MinDistance;
	static const float MaxTessellation;
	static const float MinTessellation;

private:
	void CalcAllPatchBoundsY();
	void CalcPatchBounds(UINT row, UINT col);
	void BuildQuadPatchVB();
	void BuildQuadPatchIB();
	void CalcNormalVector(TerrainCP* pVertices);
	void CalcTangentVector(TerrainCP* pVertices);

private:
	D3DXMATRIX world;
	ID3D11Buffer* cBuffer;

	class TessTerrain* terrain;

	ID3D11Buffer* quadPatchVB;
	ID3D11Buffer* quadPatchIB;

	Texture* terrainMap;
	ID3D11ShaderResourceView* terrainMapSRV;
	ID3D11ShaderResourceView* blendMapSRV;
	ID3D11ShaderResourceView* heightMapSRV;

	UINT patchVerticesCount;
	UINT patchQuadFacesCount;
	UINT patchVertexRows;
	UINT patchVertexCols;

	Material* material;
	vector<D3DXVECTOR2> patchBoundsY;

	vector<VertexColor> bvhVertices;
	vector<WORD> bvhIndices;
	ID3D11Buffer* bvhVertexBuffer;
	ID3D11Buffer* bvhIndexBuffer;

	UINT aabbCount;

	class Frustum* frustum;

	ID3DX11EffectShaderResourceVariable* terrainMapVariable;
	ID3DX11EffectShaderResourceVariable* heightMapVariable;
	ID3DX11EffectShaderResourceVariable* blendMapArrayVariable;

	ID3DX11EffectConstantBuffer* terrainVariable;

private:
	struct Buffer
	{
		D3DXCOLOR FogColor;
		float FogStart;
		float FogRange;

		float MaxDistance;
		float MinDistance;
		float MaxTessellation;
		float MinTessellation;

		float TexelCellSpaceU;
		float TexelCellSpaceV;
		float WorldCellSpace;

		D3DXVECTOR2 TexScale = D3DXVECTOR2(1, 1);
		float Padding;

		D3DXPLANE WorldFrustumPlanes[6];
	}; //buffer;

	Buffer buffer;
};