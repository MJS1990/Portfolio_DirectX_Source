#include "Framework.h"
#include "Keyboard.h"

Keyboard* Keyboard::instance = NULL;

Keyboard * Keyboard::Get()
{
	assert(instance != NULL);

	return instance;
}

void Keyboard::Create()
{
	assert(instance == NULL);

	instance = new Keyboard();
}

void Keyboard::Delete()
{
	SAFE_DELETE(instance);
}

Keyboard::Keyboard()
{
	ZeroMemory(keyState, sizeof(keyState));
	ZeroMemory(keyOldState, sizeof(keyOldState));
	ZeroMemory(keyMap, sizeof(keyMap));
}

Keyboard::~Keyboard()
{
}

void Keyboard::Update()
{
	memcpy(keyOldState, keyState, sizeof(keyOldState));

	ZeroMemory(keyState, sizeof(keyState));
	ZeroMemory(keyMap, sizeof(keyMap));

	GetKeyboardState(keyState);

	for (DWORD i = 0; i < MAX_INPUT_KEY; i++)
	{
		byte key = keyState[i] & 0x80;
		keyState[i] = key ? 1 : 0;

		int oldState = keyOldState[i];
		int state = keyState[i]; 

		if (oldState == 0 && state == 1) //이전-입력X 현재-입력
			keyMap[i] = KEY_INPUT_STATUS_DOWN;
		else if (oldState == 1 && state == 0) //이전-입력 현재-입력X
			keyMap[i] = KEY_INPUT_STATUS_UP; 
		else if (oldState == 1 && state == 1) //이전-입력 현재-입력
			keyMap[i] = KEY_INPUT_STATUS_PRESS;
		else
			keyMap[i] = KEY_INPUT_STATUS_NONE;
	}
}

