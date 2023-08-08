#pragma once

#define MAX_INPUT_MOUSE 8

class Mouse
{
public:
	//TODO : SetHandle 현재 사용X 추후 삭제 결정
	void SetHandle(HWND handle)
	{
		this->handle = handle;
	}

	static Mouse* Get();

	static void Create();
	static void Delete();

	void Update();

	LRESULT InputProc(UINT message, WPARAM wParam, LPARAM lParam);

	D3DXVECTOR3 GetPosition() { return position; }

	void Drag();
	bool TerrainPicked(OUT D3DXVECTOR3* out, class Ray& ray, float width, float height, const D3DXVECTOR3* tPosition); //OUT UINT * pickedIndex

	bool Down(DWORD button)
	{
		return buttonMap[button] == BUTTON_INPUT_STATUS_DOWN;
	}

	bool Up(DWORD button)
	{
		return buttonMap[button] == BUTTON_INPUT_STATUS_UP;
	}

	bool Press(DWORD button)
	{
		return buttonMap[button] == BUTTON_INPUT_STATUS_PRESS;
	}

	D3DXVECTOR3 GetMoveValue()
	{
		return wheelMoveValue;
	}

private:
	Mouse();
	~Mouse();

	static Mouse* instance;

	HWND handle;
	D3DXVECTOR3 position;

	byte buttonStatus[MAX_INPUT_MOUSE];
	byte buttonOldStatus[MAX_INPUT_MOUSE];
	byte buttonMap[MAX_INPUT_MOUSE];

	D3DXVECTOR3 wheelStatus;
	D3DXVECTOR3 wheelOldStatus;
	D3DXVECTOR3 wheelMoveValue;

	DWORD timeDblClk; 
	DWORD startDblClk[MAX_INPUT_MOUSE];
	int buttonCount[MAX_INPUT_MOUSE];

	//마우스 드래그 위치값
	D3DXVECTOR2 dragStartPos;
	D3DXVECTOR2 dragCurrentPos;

	enum
	{
		MOUSE_ROTATE_NONE = 0,
		MOUSE_ROTATION_LEFT,
		MOUSE_ROTATION_RIGHT
	};

	enum
	{
		BUTTON_INPUT_STATUS_NONE = 0,
		BUTTON_INPUT_STATUS_DOWN,
		BUTTON_INPUT_STATUS_UP,
		BUTTON_INPUT_STATUS_PRESS,
		BUTTON_INPUT_STATUS_DBLCLK
	};

private: //피킹 변수
	D3DXVECTOR3 start, direction;
	D3DXMATRIX world, view, projection;

	class Ray* ray;
	UINT pickedIndex;

};