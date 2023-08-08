#pragma once
#include "Systems/IExecute.h"

class TestGeometry : public IExecute
{
public:
	TestGeometry();
	~TestGeometry();

	void Initialize() override;
	void Ready() override;
	void Destroy() override;

	void Update() override;

	void PreRender() override;
	void Render() override;
	void PostRender() override;

	void ResizeScreen() override;


private:
	class Terrain* terrain;

	Shader* shader;
	ID3D11Buffer* vertexBuffer;
	TextureArray* textureArray;
};