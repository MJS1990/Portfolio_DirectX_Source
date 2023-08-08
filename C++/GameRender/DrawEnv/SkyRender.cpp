#include "stdafx.h"
#include "SkyRender.h"
#include "Viewer/Frustum.h"
#include "Environment/Sky.h"
#include "Environment/Terrain.h"
#include "Environment/QuadTree.h"
#include "Environment/Water.h"

#include "Environment/Billboard.h"
#include "Environment/TestBillboard.h"

#include "Editor/Brush.h"

//Ŀ�ǵ�����
#include "Pattern/BillboardInvoker.h"
#include "Pattern/BillboardCommand.h"

SkyRender::SkyRender()
{
	frustum = new Frustum(1000, Context::Get()->GetMainCamera(), Context::Get()->GetPerspective());
	sSky = new Sky();

	terrain = new Terrain();

	//�޸� ������ ����
	quadTree = new QuadTree(terrain);

	//��
	texture = new Texture(Contents + L"HeightMaps/heightMap256.dds");
	water = new Water(128, 128, 2.0f, texture);

	//������ + Ŀ�ǵ����� �ν��Ͻ� ����
	billboard = new Billboard(terrain);
	//bllboard��ü�� Ŀ�ǵ�ȭ ��
	//Ŀ�ǵ����Ͽ� ���� ��ü�� ����� ����� ������ ���� ��ü�� Ŀ�ǵ�ȭ �ؼ�
	//�κ�Ŀ�� SetCommand�Լ��� �Ű������� ������
	//�κ�Ŀ��ü�� Ŀ�ǵ�ȭ�� ��ü�� ��ɵ��� ����
	//Billboard -> BillboardCommand(Command���) -> BillboardInvoker -> SkyRender
	/*Billboard*/Command* bCommand = new BillboardCommand(billboard);
	bInvoker = new BillboardInvoker();	
	bInvoker->SetCommand(bCommand);
}

SkyRender::~SkyRender()
{
	SAFE_DELETE(sSky);
	SAFE_DELETE(terrain);
	SAFE_DELETE(quadTree);

	SAFE_DELETE(frustum);

	SAFE_DELETE(water);
	SAFE_DELETE(texture);

	SAFE_DELETE(billboard);
}

void SkyRender::Initialize()
{
	quadTree->Initialize();
	water->Initialize();
	sSky->Initialize();
}

void SkyRender::Ready()
{
	water->Ready();
	sSky->Ready();
}

void SkyRender::Destroy()
{
	sSky->Destroy();
}

void SkyRender::Update()
{
	sSky->Update();

	quadTree->Update();
	water->Update();

	bInvoker->BillboardUndo();
}

void SkyRender::PreRender()
{
	sSky->PreRender();
}

void SkyRender::Render()
{
	sSky->Render();

	quadTree->Render();

	bInvoker->DrawBillboard();

	water->Render();
}

void SkyRender::PostRender()
{
	sSky->PostRender();
}

void SkyRender::ResizeScreen()
{
}
