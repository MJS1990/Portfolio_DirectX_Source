#pragma once

class Frustum
{
public:
	Frustum(float zFar, class Camera* camera = NULL, class Perspective* perspective = NULL);
	~Frustum();

	void ZFar(float val) { zFar = val; }
	void Update();

	void GetPlanes(OUT D3DXPLANE* plane);

	//점이 영역안에 들어왔는지 판단
	bool ContainPoint(D3DXVECTOR3& position);
	bool ContainRect(float xCenter, float yCenter, float zCenter, float xSize, float ySize, float zSize);
	bool ContainRect(D3DXVECTOR3 center, D3DXVECTOR3 size);
	bool ContainCube(D3DXVECTOR3& center, float radius);

private:
	D3DXPLANE planes[6]; //절두체의 각 면
	
	//z간 거리를 저장하기 위한 변수, near는 시스템 절두체의 기본값 사용
	float zFar; 

	class Camera* camera;
	class Perspective* perspective;

public:
	float offsetNF = 20.0f;
	float offsetTB = 25.0f;
	float offsetLR = 20.0f;
};