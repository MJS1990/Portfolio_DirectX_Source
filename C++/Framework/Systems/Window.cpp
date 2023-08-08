#include "Framework.h"
#include "Window.h"
#include "IExecute.h"

IExecute* Window::mainExecute = NULL;

WPARAM Window::Run(IExecute * main)
{
	mainExecute = main;
	Create();

	D3DDesc desc;
	D3D::GetDesc(&desc);

	D3D::Create();
	DirectWrite::Create();
	Keyboard::Create();
	Mouse::Create();
	
	Time::Create();
	Time::Get()->Start();

	ImGui::Create(desc.Handle, D3D::GetDevice(), D3D::GetDC());
	ImGui::StyleColorsDark();

	mainExecute->Initialize();
	mainExecute->Ready(); //TODO : Main의 Ready콜 추가->콜 시점 수정 필요할수도 있음

	MSG msg = { 0 };
	
	while (true) //TODO : 윈도우실행 메인 루프, 종료 메시지 들어올때까지 반복
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			MainRender();
		}
	}
	mainExecute->Destroy();

	ImGui::Delete();
	Time::Delete();
	Mouse::Delete();
	Keyboard::Delete();
	DirectWrite::Delete();
	D3D::Delete();

	Destroy();

	return msg.wParam;
}

void Window::Create()
{
	D3DDesc desc;
	D3D::GetDesc(&desc);

	WNDCLASSEX wndClass;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wndClass.hIconSm = wndClass.hIcon;
	wndClass.hInstance = desc.Instance;
	wndClass.lpfnWndProc = (WNDPROC)WndProc;
	wndClass.lpszClassName = desc.AppName.c_str();
	wndClass.lpszMenuName = NULL;
	wndClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndClass.cbSize = sizeof(WNDCLASSEX);

	WORD wHr = RegisterClassEx(&wndClass);
	assert(wHr != 0);

	if (desc.bFullScreen == true)
	{
		DEVMODE devMode = { 0 };
		devMode.dmSize = sizeof(DEVMODE);
		devMode.dmPelsWidth = (DWORD)desc.Width;
		devMode.dmPelsHeight = (DWORD)desc.Height;
		devMode.dmBitsPerPel = 32;
		devMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		ChangeDisplaySettings(&devMode, CDS_FULLSCREEN);
	}

	desc.Handle = CreateWindowEx
	(
		WS_EX_APPWINDOW
		, desc.AppName.c_str()
		, desc.AppName.c_str()
		, WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW
		, CW_USEDEFAULT
		, CW_USEDEFAULT
		, CW_USEDEFAULT
		, CW_USEDEFAULT
		, NULL
		, (HMENU)NULL
		, desc.Instance
		, NULL
	);
	assert(desc.Handle != NULL);
	D3D::SetDesc(desc);

	RECT rect = { 0, 0, (LONG)desc.Width, (LONG)desc.Height };

	//GetSystemMetrics(SM_CXSCREEN) 화면 수평해상도 반환
	//GetSystemMetrics(SM_CYSCREEN) 화면 수직해상도 반환
	UINT centerX = (GetSystemMetrics(SM_CXSCREEN) - (UINT)desc.Width) / 2;
	UINT centerY = (GetSystemMetrics(SM_CYSCREEN) - (UINT)desc.Height) / 2;

	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
	MoveWindow
	(
		desc.Handle
		, centerX, centerY
		, rect.right - rect.left, rect.bottom - rect.top
		, TRUE
	);
	ShowWindow(desc.Handle, SW_SHOWNORMAL);
	SetForegroundWindow(desc.Handle);
	SetFocus(desc.Handle);

	ShowCursor(true);
}

void Window::Destroy()
{
	D3DDesc desc;
	D3D::GetDesc(&desc);

	if (desc.bFullScreen == true)
		ChangeDisplaySettings(NULL, 0);

	DestroyWindow(desc.Handle);

	UnregisterClass(desc.AppName.c_str(), desc.Instance);
}

void Window::MainRender()
{
	Time::Get()->Update();

	if (ImGui::IsMouseHoveringAnyWindow() == false)
	{
		Keyboard::Get()->Update();
		Mouse::Get()->Update();
	}

	ImGui::Update();

	mainExecute->Update();
	mainExecute->PreRender();

	D3D::Get()->SetRenderTarget();
	D3D::Get()->Clear(D3DXCOLOR(0.53f, 0.81f, 0.91f, 1)); //D3DXCOLOR(1, 1, 1, 1)
	{
		mainExecute->Render();
		ImGui::Render();

		DirectWrite::GetDC()->BeginDraw();
		{
			mainExecute->PostRender();
		}
		DirectWrite::GetDC()->EndDraw();
	}
	D3D::Get()->Present();
}

LRESULT Window::WndProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
	Mouse::Get()->InputProc(message, wParam, lParam);

	if (ImGui::WndProc((UINT*)handle, message, wParam, lParam))
		return true;

	if (message == WM_SIZE)
	{
		ImGui::Invalidate();
		{
			if (mainExecute != NULL)
			{
				float width = (float)LOWORD(lParam);
				float height = (float)HIWORD(lParam);

				if (DirectWrite::Get() != NULL)
					DirectWrite::DeleteSurface();

				if (D3D::Get() != NULL)
					D3D::Get()->ResizeScreen(width, height);

				if (DirectWrite::Get() != NULL)
					DirectWrite::CreateSurface();

				mainExecute->ResizeScreen();
			}
		}
		ImGui::Validate();
	}

	if (message == WM_CLOSE || message == WM_DESTROY)
	{
		PostQuitMessage(0);

		return 0;
	}
	//DefWindowProc - 나머지 메시지 처리
	return DefWindowProc(handle, message, wParam, lParam);
}
