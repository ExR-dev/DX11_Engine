
#include <Windows.h>
#include <d3d11.h>

#include "ErrMsg.h"
#include "WindowHelper.h"
#include "Game.h"
#include "Time.h"
#include "Input.h"


int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	OutputDebugString(L"\n=======| Start |===========================================================================\n");

	constexpr UINT WIDTH = 1920;
	constexpr UINT HEIGHT = 1080;
	HWND window;

	if (!SetupWindow(hInstance, WIDTH, HEIGHT, nCmdShow, window))
	{
		ErrMsg("Failed to setup window!");
		return -1;
	}

	Game game = { };
	if (!game.Setup(WIDTH, HEIGHT, window))
	{
		ErrMsg("Failed to setup graphics!");
		return -1;
	}

	Scene scene = { };
	if (!game.SetScene(&scene))
	{
		ErrMsg("Failed to set scene!");
		return -1;
	}
	
	Time time = { };
	Input input = { };
	MSG msg = { };

	while (!(GetKeyState(VK_ESCAPE) & 0x8000) && msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		time.Update();

		if (!input.Update(window))
		{
			ErrMsg("Failed to update input!");
			return -1;
		}

		if (input.GetKey(KeyCode::Tab) == KeyState::Pressed)
			input.ToggleLockCursor();

		if (input.GetKey(KeyCode::CapsLock) == KeyState::Pressed)
			input.ToggleShowCursor();

		if (!game.Update(time, input))
		{
			ErrMsg("Failed to update game logic!");
			return -1;
		}

		if (!game.Render(time))
		{
			ErrMsg("Failed to render frame!");
			return -1;
		}
	}

	OutputDebugString(L"========| End |============================================================================\n\n");
	return 0;
}
