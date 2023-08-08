#include "Framework.h"
#include "Mouse.h"

Mouse* Mouse::instance = NULL;

Mouse * Mouse::Get()
{
	return instance;
}

void Mouse::Create()
{
	assert(instance == NULL);

	instance = new Mouse();
}

void Mouse::Delete()
{
	SAFE_DELETE(instance);
}

Mouse::Mouse()
	: position(0.0f, 0.0f, 0.0f)
	, wheelStatus(0.0f, 0.0f, 0.0f), wheelOldStatus(0.0f, 0.0f, 0.0f), wheelMoveValue(0.0f, 0.0f, 0.0f)
{
	//초기화///////////////////////////////////////////////////////////////////
	ZeroMemory(buttonStatus, sizeof(byte) * MAX_INPUT_MOUSE);
	ZeroMemory(buttonOldStatus, sizeof(byte) * MAX_INPUT_MOUSE);
	ZeroMemory(buttonMap, sizeof(byte) * MAX_INPUT_MOUSE);

	ZeroMemory(startDblClk, sizeof(DWORD) * MAX_INPUT_MOUSE);
	ZeroMemory(buttonCount, sizeof(int) * MAX_INPUT_MOUSE);

	timeDblClk = GetDoubleClickTime(); //TODO : GetDoubleClickTime()함수 리턴형 UINT
	startDblClk[0] = GetTickCount();

	for (int i = 1; i < MAX_INPUT_MOUSE; i++)
		startDblClk[i] = startDblClk[0];

	DWORD tLine = 0;
	SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &tLine, 0); //휠스크롤시에 이동하는 줄의 수 리턴(기본값 3)
	///////////////////////////////////////////////////////////////////////////

	dragStartPos = dragCurrentPos = D3DXVECTOR2(0.0f, 0.0f);
}

Mouse::~Mouse()
{
}

void Mouse::Update()
{
	memcpy(buttonOldStatus, buttonStatus, sizeof(buttonOldStatus));

	ZeroMemory(buttonStatus, sizeof(buttonStatus));
	ZeroMemory(buttonMap, sizeof(buttonMap));

	//GetAsyncKeyState()->버튼의 up, down시 호출
	//키의 상태를 리턴
	//리턴값 0x8000 -> 이전에 누른적이 없고 호출시점에는 눌려있는 상태
	buttonStatus[0] = GetAsyncKeyState(VK_LBUTTON) & 0x8000 ? 1 : 0;
	buttonStatus[1] = GetAsyncKeyState(VK_RBUTTON) & 0x8000 ? 1 : 0;
	buttonStatus[2] = GetAsyncKeyState(VK_MBUTTON) & 0x8000 ? 1 : 0;

	for (DWORD i = 0; i < MAX_INPUT_MOUSE; i++)
	{
		int tOldStatus = buttonOldStatus[i];
		int tStatus = buttonStatus[i];

		if (tOldStatus == 0 && tStatus == 1) //이전에 안눌렸고 지금 눌렸다
			buttonMap[i] = BUTTON_INPUT_STATUS_DOWN;
		else if (tOldStatus == 1 && tStatus == 0) //이전에 눌렸고 지금 안누렸다
			buttonMap[i] = BUTTON_INPUT_STATUS_UP;
		else if (tOldStatus == 1 && tStatus == 1) //이전에 눌렸고 지금도 눌려있다
			buttonMap[i] = BUTTON_INPUT_STATUS_PRESS;
		else //이전도 지금도 눌려있지 않다
			buttonMap[i] = BUTTON_INPUT_STATUS_NONE;
	}

	POINT point;
	GetCursorPos(&point); //전체화면 기준
	ScreenToClient(handle, &point); //화면에 맞게 좌표변환

	wheelOldStatus.x = wheelStatus.x;
	wheelOldStatus.y = wheelStatus.y;

	wheelStatus.x = float(point.x);
	wheelStatus.y = float(point.y);

	wheelMoveValue = wheelStatus - wheelOldStatus;
	wheelOldStatus.z = wheelStatus.z;

	DWORD tButtonStatus = GetTickCount();
	for (DWORD i = 0; i < MAX_INPUT_MOUSE; i++)
	{
		if (buttonMap[i] == BUTTON_INPUT_STATUS_DOWN) 
		{
			if (buttonCount[i] == 1)
			{
				if ((tButtonStatus - startDblClk[i]) >= timeDblClk)
					buttonCount[i] = 0;
			}
			buttonCount[i]++;

			if (buttonCount[i] == 1)
				startDblClk[i] = tButtonStatus;
		}

		if (buttonMap[i] == BUTTON_INPUT_STATUS_UP)
		{
			if (buttonCount[i] == 1) //한번 클릭
			{
				if ((tButtonStatus - startDblClk[i]) >= timeDblClk)
					buttonCount[i] = 0;
			}
			else if (buttonCount[i] == 2) //더블 클릭
			{
				if ((tButtonStatus - startDblClk[i]) <= timeDblClk) //더블클릭 조건
					buttonMap[i] = BUTTON_INPUT_STATUS_DBLCLK;

				buttonCount[i] = 0;
			}
		}//if
	}//for(i)
}

LRESULT Mouse::InputProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_LBUTTONDOWN || message == WM_MOUSEMOVE) //클릭, 이동 메시지가 들어왔을 경우
	{
		position.x = (float)LOWORD(lParam); //하위 word에 x
		position.y = (float)HIWORD(lParam); //상위 word에 y
	}

	if (message == WM_MOUSEWHEEL) //휠입력 메시지가 들어왔을 경우
	{
		short tWheelValue = (short)HIWORD(wParam);

		wheelOldStatus.z = wheelStatus.z;
		wheelStatus.z += (float)tWheelValue;
	}

	return TRUE;
}

void Mouse::Drag()
{
	if (Press(1))
		dragStartPos = D3DXVECTOR2(GetPosition().x, GetPosition().y);

	if (dragStartPos != D3DXVECTOR2(GetPosition().x, GetPosition().y))
		dragCurrentPos = D3DXVECTOR2(GetPosition().x, GetPosition().y);

}

//bool Mouse::TerrainPicked(OUT D3DXVECTOR3 * out, Ray & ray, float width, float height, const D3DXVECTOR3* tPosition) //OUT UINT * pickedIndex, 이전 마지막 파라메터값 const D3DXVECTOR3* tPosition
//{
//	Context::Get()->GetMainCamera()->Position(&start);
//
//	//TODO : Ray추가
//	Context::Get()->GetMainCamera()->Matrix(&view);
//	Context::Get()->GetPerspective()->GetMatrix(&projection);
//	//World공간까지 역변환된 3차원 마우스 좌표
//	Context::Get()->GetViewport()->GetRay(&ray, start, world, view, projection);
//
//	direction = ray.Direction;
//
//	for (UINT z = 0; z < height; z++)
//	{
//		for (UINT x = 0; x < width; x++)
//		{
//			int i = x * 6;
//			UINT index[4];
//			index[0] = (((width * 6) * z)) + i;
//			index[1] = (((width * 6) * z)) + i + 1;
//			index[2] = (((width * 6) * z)) + i + 2;
//			index[3] = (((width * 6) * z)) + i + 5;
//
//			D3DXVECTOR3 p[4];
//			for (int i = 0; i < 4; i++)
//				p[i] = tPosition[index[i]];
//
//			float u, v, distance;
//			if (D3DXIntersectTri(&p[0], &p[1], &p[2], &ray.Position, &direction, &u, &v, &ray.Distance))
//			{
//				*out = p[0] + (p[1] - p[0]) * u + (p[2] - p[0]) * v;
//				//*pickedIndex = index[0] / 6;
//				return true;
//			}
//
//			if (D3DXIntersectTri(&p[3], &p[1], &p[2], &ray.Position, &direction, &u, &v, &ray.Distance))
//			{
//				*out = p[3] + (p[1] - p[3]) * u + (p[2] - p[3]) * v;
//				//*pickedIndex = (index[3] - 5) / 6;
//				return true;
//			}
//		}
//	}
//
//	return false;
//}