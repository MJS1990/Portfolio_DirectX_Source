#pragma once

class Window
{
public:
	static WPARAM Run(class IExecute* main);
	
private:
	static void Create();
	static void Destroy();
	
	static void MainRender();

	static LRESULT CALLBACK WndProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);

private:
	static class IExecute* mainExecute;
};