#pragma once
#include "Systems/IExecute.h"

class DrawTerrain : public IExecute
{
public:
	DrawTerrain();
	~DrawTerrain();

public:
	void Initialize() override;
	void Ready() override;
	void Destroy() override;
	void Update() override;
	void PreRender() override;
	void Render() override;
	void PostRender() override;
	void ResizeScreen() override;

private:
	Material* terrainMaterial;
	class TessTerrain* terrain;
	
	class Sky* sSky;

	//test
	class Terrain* testTerrain;
	class QuadTree* quadTree;
};