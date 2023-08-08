#include "stdafx.h"
#include "Main.h"
#include "Systems/Window.h"
#include "Viewer/Freedom.h"

#include "DrawEnv/SkyRender.h"
#include "DrawEnv/RenderTessTerrain.h"
#include "Test/TestModel.h"

#include "Character/Player/Player.h"
#include"DrawEnv/DrawTerrain.h"

void Main::Initialize()
{
	Context::Create();

	//Context::Get()->GetMainCamera()->RotationDegree(20.0f, 18.0f); //지형(SkyRender)용
	//Context::Get()->GetMainCamera()->Position(47.0f, 32.0f, -17.0f); //지형(SkyRender)용
	//Context::Get()->GetMainCamera()->Position(0.0f, 11.0f, -20.0f); //TestModel용
	//Context::Get()->GetMainCamera()->RotationDegree(15.0f, 180.0f);

	Context::Get()->GetGlobalLight()->Direction = D3DXVECTOR3(-1.0f, -1.0f, -0.23f);

	((Freedom*)Context::Get()->GetMainCamera())->MoveSpeed(30.0f); //카메라 수동이동

	Push(new SkyRender());
	//Push(new DrawTerrain());
	//Push(new Player());
}

void Main::Ready()
{
	for (IExecute* exe : executes)
		exe->Ready();
}

void Main::Destroy()
{
	for (IExecute* exe : executes)
	{
		exe->Destroy();
		SAFE_DELETE(exe);
	}

	Context::Delete();
}

void Main::Update()
{
	Context::Get()->Update();

	for (IExecute* exe : executes)
		exe->Update();
}

void Main::PreRender()
{
	for (IExecute* exe : executes)
		exe->PreRender();

	ImGui::SliderFloat3("GlobalLight Direction", (float*)Context::Get()->GetGlobalLight()->Direction, -1, 1);
	ImGui::ColorEdit4("GlobalLight Diffuse", (float*)Context::Get()->GetGlobalLight()->Diffuse);
	ImGui::SliderFloat3("GlobalLight Position", (float*)Context::Get()->GetGlobalLight()->Position, -100, 100);

	Context::Get()->ChangeGlobalLight();
}

void Main::Render()
{
	Context::Get()->GetViewport()->RSSetViewport();

	for (IExecute* exe : executes)
		exe->Render();
}

void Main::PostRender()
{
	for (IExecute* exe : executes)
		exe->PostRender();

	wstring str;
	RECT rect = { 0, 0, 300, 300 };

	str = String::Format(L"FPS : %.0f", Time::Get()->FPS());
	DirectWrite::RenderText(str, rect, 12);

	rect.top += 12;
	D3DXVECTOR3 cameraPos;
	Context::Get()->GetMainCamera()->Position(&cameraPos);
	str = String::Format(L"CameraPosition : %.0f, %.0f, %.0f", cameraPos.x, cameraPos.y, cameraPos.z);
	DirectWrite::RenderText(str, rect, 12);

	rect.top += 12;
	D3DXVECTOR2 cameraRot;
	Context::Get()->GetMainCamera()->RotationDegree(&cameraRot);
	str = String::Format(L"CameraRotationDegree : %.0f, %.0f", cameraRot.x, cameraRot.y);
	DirectWrite::RenderText(str, rect, 12);
}

void Main::ResizeScreen()
{
	D3DDesc desc;
	D3D::GetDesc(&desc);

	if (Context::Get() != NULL)
	{
		Context::Get()->GetPerspective()->Set(desc.Width, desc.Height);
		Context::Get()->GetViewport()->Set(desc.Width, desc.Height);
	}

	for (IExecute* exe : executes)
		exe->ResizeScreen();
}

void Main::Push(IExecute* execute)
{
	executes.push_back(execute);

	execute->Initialize();
}

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, LPSTR param, int command)
{
	D3DDesc desc;
	desc.AppName = L"D3D Game";
	desc.Instance = instance;
	desc.bFullScreen = false;
	desc.bVsync = false;
	desc.Handle = NULL;
	desc.Width = 1280;
	desc.Height = 720;
	D3D::SetDesc(desc);

	Main* main = new Main();
	WPARAM wParam = Window::Run(main);

	SAFE_DELETE(main);

	return wParam;
}
