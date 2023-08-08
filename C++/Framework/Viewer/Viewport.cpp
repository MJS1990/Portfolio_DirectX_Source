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
	//1. bind�� ����Ʈ�� �ε���(����)
	//2. ����̽��� bind�� D3D11_VIEWPORT ����ü�� �迭
	D3D::GetDC()->RSSetViewports(1, &viewport); 
}

void Viewport::Set(float width, float height, float x, float y, float minDepth, float maxDepth)
{
	viewport.TopLeftX = this->x = x;
	viewport.TopLeftY = this->y = y;
	viewport.Width = this->width = width;
	viewport.Height = this->height = height;
	viewport.MinDepth = this->minDepth = minDepth;
	viewport.MaxDepth = this->maxDepth = maxDepth; //���� 0~1

	RSSetViewport();
}
//Picking ������
void Viewport::GetRay(Ray * ray, D3DXVECTOR3 & start, D3DXMATRIX & world, D3DXMATRIX & view, D3DXMATRIX & projection)
{
	D3DXVECTOR2 screenSize;
	screenSize.x = width;
	screenSize.y = height;

	D3DXVECTOR3 mouse = Mouse::Get()->GetPosition();

	D3DXVECTOR2 point; //����ȯ ����� ������ ����

	//Inv Viewport
	{
		point.x = ((2.0f * mouse.x) / screenSize.x) - 1.0f; //-1 ~ 1
		//���콺�� ���� ����� 0, 0
		point.y = (((2.0f * mouse.y) / screenSize.y) - 1.0f) * -1.0f; //-1 ~ 1
	}

	//Inv Projection
	{
		point.x = point.x / projection._11; //_11 -> ��������� Width��
		point.y = point.y / projection._22; //_22 -> ��������� Height��
	}

	D3DXVECTOR3 direction; //����ȯ�� �������� ����(ī�޶��� ����)
	//Inv View
	{
		D3DXMATRIX invView;
		D3DXMatrixInverse(&invView, NULL, &view);
		//���ⱸ�ϴ� �Լ�
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
