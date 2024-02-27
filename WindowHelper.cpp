#include "WindowHelper.h"

#include <iostream>
#include "ErrMsg.h"


#ifdef _DEBUG
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif // _DEBUG

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
#ifdef _DEBUG
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		return true;
#endif // _DEBUG

	// sort through and find what code to run for the message given
	switch (message)
	{
		// this message is read when the window is closed
		case WM_DESTROY:
			// close the application entirely
			PostQuitMessage(0);
			return 0;

		default:
			break;
	}

	// Handle any messages the switch statement didn't by using default methodology
	return DefWindowProc(hWnd, message, wParam, lParam);
}

bool SetupWindow(HINSTANCE instance, UINT width, UINT height, int nCmdShow, HWND& window)
{
	const wchar_t CLASS_NAME[] = L"Test Window Class";

	WNDCLASS wc = {};

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = instance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	window = CreateWindowEx(
		0, CLASS_NAME, L"TEST WINDOW", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, width, height, nullptr, nullptr, instance, nullptr
	);

	if (window == nullptr)
	{
		ErrMsg("HWND was nullptr, last error: " + std::to_string(GetLastError()));
		return false;
	}

	ShowWindow(window, nCmdShow);
	return true;
}