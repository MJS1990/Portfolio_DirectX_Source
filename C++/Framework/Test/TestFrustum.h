#pragma once

class TestFrustum
{
public:
	TestFrustum();
	~TestFrustum();

	void Initialize();
	void Ready();

	void Update();

	void Destroy();

	void PreRender() {}
	void Render();
	void PostRender() {}


private:
	UINT drawCount;
	vector<D3DXVECTOR3> positions;
	
	class MeshSphere* sphere;

	class Frustum* frustum;
	class Camera* camera;
	class Perspective* projection;

	Material* material;
	
};