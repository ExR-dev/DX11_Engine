
#include <Windows.h>
#include <iostream>
#include <d3d11.h>

#include "WindowHelper.h"
#include "ContentLoader.h"
#include "Game.h"
#include "Time.h"
#include "ErrMsg.h"


int APIENTRY wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	OutputDebugString(L"\n=======| Start |===========================================================================\n");

	/*MeshData meshData = { };

	LoadMeshFromFile("Models\\Fallback.obj", meshData);
	WriteMeshToFile("Models\\Fallback.txt", meshData);

	OutputDebugString(L"========| End |============================================================================\n\n");
	return 0;*/

	constexpr UINT WIDTH = 900;
	constexpr UINT HEIGHT = 900;
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
	MSG msg = { };

	while (!(GetKeyState(VK_ESCAPE) & 0x8000) && msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		time.Update();

		if (!game.Update(time))
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
