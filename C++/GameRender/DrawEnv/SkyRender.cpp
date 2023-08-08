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

//커맨드패턴
#include "Pattern/BillboardInvoker.h"
#include "Pattern/BillboardCommand.h"

SkyRender::SkyRender()
{
	frustum = new Frustum(1000, Context::Get()->GetMainCamera(), Context::Get()->GetPerspective());
	sSky = new Sky();

	terrain = new Terrain();

	//펄린 노이즈 지형
	quadTree = new QuadTree(terrain);

	//물
	texture = new Texture(Contents + L"HeightMaps/heightMap256.dds");
	water = new Water(128, 128, 2.0f, texture);

	//빌보드 + 커맨드패턴 인스턴스 선언
	billboard = new Billboard(terrain);
	//bllboard객체를 커맨드화 함
	//커맨드패턴에 여러 객체의 기능을 만들고 싶으면 여러 객체를 커맨드화 해서
	//인보커의 SetCommand함수에 매개변수로 대입함
	//인보커객체로 커맨드화한 객체의 기능들을 실행
	//Billboard -> BillboardCommand(Command상속) -> BillboardInvoker -> SkyRender
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
