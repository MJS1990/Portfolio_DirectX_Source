#include "Framework.h"
#include "Frustum.h"

Frustum::Frustum(float zFar, Camera * camera, Perspective * perspective)
	: zFar(zFar), camera(camera), perspective(perspective)
{
	if (camera == NULL)
		this->camera = Context::Get()->GetMainCamera();

	if (perspective == NULL) //TOOD : �߰�
		this->perspective = Context::Get()->GetPerspective();
}

Frustum::~Frustum()
{
}

void Frustum::Update()
{
	D3DXMATRIX v, p;
	camera->Matrix(&v);
	perspective->GetMatrix(&p);
	
	float zNear = -p._43 / p._33; // -far / near -> ���� frustum�� near ����
	//z�� ī�޶����� ���� ȭ������� �ݴ��̹Ƿ� ����� Far���� ������ ���� 
	float r = zFar / (zFar - zNear); //���� frustum�� far ����

	//������ projection ����� Near, Far���� ���� ����� Near(zNear), Far(r)���� ����
	p._33 = r;
	p._43 = -r * zNear;

	D3DXMATRIX m;
	//���̴������� W,V,P��ȯó�� ��ȯ�� ������� m�� ���
	D3DXMatrixMultiply(&m, &v, &p); 
	
	//���� ���������� ������ ����
	//a, b, c -> ����
	//d -> ��ġ(�Ÿ�)
	//Near
	//��ȯ�ɰ� + z��
	planes[0].a = m._14 + m._13;
	planes[0].b = m._24 + m._23;
	planes[0].c = m._34 + m._33;
	planes[0].d = m._44 + m._43 + offsetNF;
	

	//Far
	//��ȯ�ɰ� - z��
	planes[1].a = m._14 - m._13;
	planes[1].b = m._24 - m._23;
	planes[1].c = m._34 - m._33;
	planes[1].d = m._44 - m._43 + offsetNF;
	

	//Left
	//��ȯ�ɰ� + x��
	planes[2].a = m._14 + m._11;
	planes[2].b = m._24 + m._21;
	planes[2].c = m._34 + m._31;
	planes[2].d = m._44 + m._41 + offsetLR;

	//Right
	//��ȯ�ɰ� - x�� -> �������� �����Ƿ� �������� -
	planes[3].a = m._14 - m._11;
	planes[3].b = m._24 - m._21;
	planes[3].c = m._34 - m._31;
	planes[3].d = m._44 - m._41 + offsetLR;


	//Top
	//��ȯ�ɰ� - y�� -> �������� �����Ƿ� ������ -
	planes[4].a = m._14 - m._12;
	planes[4].b = m._24 - m._22;
	planes[4].c = m._34 - m._32;
	planes[4].d = m._44 - m._42 + offsetTB;

	//Bottom
	//��ȯ�ɰ� + y��
	planes[5].a = m._14 + m._12;
	planes[5].b = m._24 + m._22;
	planes[5].c = m._34 + m._32;
	planes[5].d = m._44 + m._42 +  offsetTB;

	for (int i = 0; i < 6; i++)
		D3DXPlaneNormalize(&planes[i], &planes[i]);
}

void Frustum::GetPlanes(OUT D3DXPLANE * plane)
{
	D3DXMATRIX v, p;
	camera->Matrix(&v);
	perspective->GetMatrix(&p);

	//�߰�
	float zMin, r;
	zMin = -p._43 / p._33;
	r = 1000.0f / (1000.0f - zMin);
	p._33 = r;
	p._43 = -r * zMin;

	D3DXMATRIX m;
	D3DXMatrixMultiply(&m, &v, &p);

	//Left
	plane[0].a = m._14 + m._11;
	plane[0].b = m._24 + m._21;
	plane[0].c = m._34 + m._31;
	plane[0].d = m._44 + m._41;

	//Right
	plane[1].a = m._14 - m._11;
	plane[1].b = m._24 - m._21;
	plane[1].c = m._34 - m._31;
	plane[1].d = m._44 - m._41;

	//Bottom
	plane[2].a = m._14 + m._12;
	plane[2].b = m._24 + m._22;
	plane[2].c = m._34 + m._32;
	plane[2].d = m._44 + m._42;

	//Top
	plane[3].a = m._14 - m._12;
	plane[3].b = m._24 - m._22;
	plane[3].c = m._34 - m._32;
	plane[3].d = m._44 - m._42;

	//Near
	plane[4].a = m._13 + m._13;
	plane[4].b = m._23 + m._23;
	plane[4].c = m._33 + m._33;
	plane[4].d = m._44 + m._43;

	//Far
	plane[5].a = m._14 - m._13;
	plane[5].b = m._24 - m._23;
	plane[5].c = m._34 - m._33;
	plane[5].d = m._44 - m._43;
	
	for (int i = 0; i < 6; i++)
		D3DXPlaneNormalize(&planes[i], &plane[i]);
}

bool Frustum::ContainPoint(D3DXVECTOR3 & position)
{
	//position.y += 60.0f;
	//D3DXVECTOR3 temp = D3DXVECTOR3(position.x, camera->GetPosition().y, position.z);

	for (int i = 0; i < 6; i++)
	{
		//���� ��ġ�� ���(������)�̸� ���� �ִ°�
		//frusrum 6�� �鿡 ���ؼ� ���� �˻縦 ����
		if (D3DXPlaneDotCoord(&planes[i], &position) < 0.0f)
			return false;

		//TODO : ��ġ�� ���������� ����, �����ʿ��ϸ� ����
		//if (D3DXPlaneDotCoord(&planes[i], &temp) < 0.0f)
		//	return false;
	}

	//position.y -= 60.0f;

	return true;
}

bool Frustum::ContainRect(float xCenter, float yCenter, float zCenter, float xSize, float ySize, float zSize)
{
	for (int i = 0; i < 6; i++)
	{
		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3((xCenter - xSize), (yCenter - ySize), (zCenter - zSize))) >= 0.0f)
			continue;

		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3((xCenter + xSize), (yCenter - ySize), (zCenter - zSize))) >= 0.0f)
			continue;

		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3((xCenter - xSize), (yCenter + ySize), (zCenter - zSize))) >= 0.0f)
			continue;

		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3((xCenter - xSize), (yCenter - ySize), (zCenter + zSize))) >= 0.0f)
			continue;

		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3((xCenter + xSize), (yCenter + ySize), (zCenter - zSize))) >= 0.0f)
			continue;

		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3((xCenter + xSize), (yCenter - ySize), (zCenter + zSize))) >= 0.0f)
			continue;

		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3((xCenter - xSize), (yCenter + ySize), (zCenter + zSize))) >= 0.0f)
			continue;

		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3((xCenter + xSize), (yCenter + ySize), (zCenter + zSize))) >= 0.0f)
			continue;

		return false;
	}

	return true;
}

bool Frustum::ContainRect(D3DXVECTOR3 center, D3DXVECTOR3 size)
{
	return ContainRect(center.x, center.y, center.z, size.x, size.y, size.z);
}

bool Frustum::ContainCube(D3DXVECTOR3 & center, float radius)
{
	D3DXVECTOR3 check;

	for (int i = 0; i < 6; i++)
	{
		check.x = center.x - radius;
		check.y = center.y - radius;
		check.z = center.z - radius;
		if (D3DXPlaneDotCoord(&planes[i], &check) >= 0.0f)
			continue;

		check.x = center.x + radius;
		check.y = center.y - radius;
		check.z = center.z - radius;
		if (D3DXPlaneDotCoord(&planes[i], &check) >= 0.0f)
			continue;

		check.x = center.x - radius;
		check.y = center.y + radius;
		check.z = center.z - radius;
		if (D3DXPlaneDotCoord(&planes[i], &check) >= 0.0f)
			continue;

		check.x = center.x + radius;
		check.y = center.y + radius;
		check.z = center.z - radius;
		if (D3DXPlaneDotCoord(&planes[i], &check) >= 0.0f)
			continue;

		check.x = center.x - radius;
		check.y = center.y - radius;
		check.z = center.z + radius;
		if (D3DXPlaneDotCoord(&planes[i], &check) >= 0.0f)
			continue;

		check.x = center.x + radius;
		check.y = center.y - radius;
		check.z = center.z + radius;
		if (D3DXPlaneDotCoord(&planes[i], &check) >= 0.0f)
			continue;

		check.x = center.x - radius;
		check.y = center.y + radius;
		check.z = center.z + radius;
		if (D3DXPlaneDotCoord(&planes[i], &check) >= 0.0f)
			continue;

		check.x = center.x + radius;
		check.y = center.y + radius;
		check.z = center.z + radius;
		if (D3DXPlaneDotCoord(&planes[i], &check) >= 0.0f)
			continue;

		return false;
	}

	return true;
}
