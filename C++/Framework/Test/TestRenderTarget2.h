#pragma once

class TestRenderTarget2
{
public:
	TestRenderTarget2();
	~TestRenderTarget2();

	void Initialize();
	void Ready();

	void Destroy();

	void Update();

	void PreRender();
	void Render();
	void PostRender();

private:
	//메인 화면 랜더링용
	Material* material;
	ID3D11Buffer* vertexBuffer, *indexBuffer;
	VertexTexture* vertices;
	UINT* indices;

	D3DXMATRIX world;

	int vertexCount, indexCount;

	Texture* texture;
	
	//Test RenderTarget
	RenderTargetView* rtv;
	Render2D* render2d;

	Viewport* rtvViewport;
	ID3D11SamplerState* targetSampler;
};