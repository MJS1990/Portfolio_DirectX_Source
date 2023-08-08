#include "Framework.h"
#include "Viewport.h"

Viewport::Viewport(UINT width, UINT height, float x, float y, float minDepth, float maxDepth)
{
	Set(static_cast<float>(width), static_cast<float>(height), x, y, minDepth, maxDepth);
}

Viewport::~Viewport()
{
}

void Viewport::RSSetViewport()
{
	//1. bind할 뷰포트의 인덱스(갯수)
	//2. 디바이스에 bind할 D3D11_VIEWPORT 구조체의 배열
	D3D::GetDC()->RSSetViewports(1, &viewport); 
}

void Viewport::Set(float width, float height, float x, float y, float minDepth, float maxDepth)
{
	viewport.TopLeftX = this->x = x;
	viewport.TopLeftY = this->y = y;
	viewport.Width = this->width = width;
	viewport.Height = this->height = height;
	viewport.MinDepth = this->minDepth = minDepth;
	viewport.MaxDepth = this->maxDepth = maxDepth; //범위 0~1

	RSSetViewport();
}
//Picking 반직선
void Viewport::GetRay(Ray * ray, D3DXVECTOR3 & start, D3DXMATRIX & world, D3DXMATRIX & view, D3DXMATRIX & projection)
{
	D3DXVECTOR2 screenSize;
	screenSize.x = width;
	screenSize.y = height;

	D3DXVECTOR3 mouse = Mouse::Get()->GetPosition();

	D3DXVECTOR2 point; //역변환 결과값 저장할 변수

	//Inv Viewport
	{
		point.x = ((2.0f * mouse.x) / screenSize.x) - 1.0f; //-1 ~ 1
		//마우스는 좌측 상단이 0, 0
		point.y = (((2.0f * mouse.y) / screenSize.y) - 1.0f) * -1.0f; //-1 ~ 1
	}

	//Inv Projection
	{
		point.x = point.x / projection._11; //_11 -> 투영행렬의 Width값
		point.y = point.y / projection._22; //_22 -> 투영행렬의 Height값
	}

	D3DXVECTOR3 direction; //역변환된 반직선의 방향(카메라의 방향)
	//Inv View
	{
		D3DXMATRIX invView;
		D3DXMatrixInverse(&invView, NULL, &view);
		//방향구하는 함수
		D3DXVec3TransformNormal(&direction, &D3DXVECTOR3(point.x, point.y, 1.0f), &invView);
		D3DXVec3Normalize(&direction, &direction);
	}

	//Inv World
	D3DXVECTOR3 tempStart;
	{
		D3DXMATRIX invWorld;
		D3DXMatrixInverse(&invWorld, NULL, &world);
	
		D3DXVec3TransformCoord(&tempStart, &start, &invWorld);
		D3DXVec3TransformNormal(&direction, &direction, &invWorld);
		D3DXVec3Normalize(&direction, &direction);
	}

	ray->Position = tempStart;
	ray->Direction = direction;
}
