#pragma once

class Viewport
{
public:
	Viewport(UINT width, UINT height, float x = 0.0f, float y = 0.0f, float minDepth = 0.0f, float maxDepth = 1.0f);
	~Viewport();

	void RSSetViewport();
	void Set(float width, float height, float x = 0.0f, float y = 0.0f, float minDepth = 0.0f, float maxDepth = 1.0f);

	float GetWidth() { return width; }
	float GetHeight() { return height; }

	void GetRay(class Ray* ray, D3DXVECTOR3& start, D3DXMATRIX& world, D3DXMATRIX& view, D3DXMATRIX& projection);

private:
	float x, y;
	float width, height;
	float minDepth, maxDepth;

	D3D11_VIEWPORT viewport;
};