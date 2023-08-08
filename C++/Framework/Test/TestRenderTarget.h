#pragma once

class TestRenderTarget
{
public:
	TestRenderTarget();
	~TestRenderTarget();

	void Initialize();
	void Ready();

	void Destroy();

	void Update();

	void PreRender();
	void Render();
	void PostRender();

private:
	MeshQuad* quad;
	Material* material;

	ID3D11Buffer* vertexBuffer, *indexBuffer;
	VertexTexture* vertices;
	UINT* indices;

	D3DXMATRIX world;

	int vertexCount, indexCount;

	Texture* texture;

	//Test RenderTarget
	RenderTargetView* rtv1;
	Render2D* render1;

	Viewport* rtvViewport;
};