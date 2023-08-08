#pragma once
#include "Systems/IExecute.h"

class SkyRender : public IExecute
{
public:
	SkyRender();
	~SkyRender();

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
	class Sky* sSky;
	class Terrain* terrain;
	class QuadTree* quadTree;
	class Frustum* frustum;

	class Water* water;

	class Billboard* billboard;
	class BillboardInvoker* bInvoker;
	
	Texture* texture;
};