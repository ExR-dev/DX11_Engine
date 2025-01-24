// Press Esc to quit.
// Move by focusing the window and pressing Tab, this locks the cursor. After that you can use WASD to move.
// Caps Lock hides the mouse.
// Pressing Middle Mouse Button selects an entity in front of the camera.
// Pressing 1-9 selects one of the first 9 entities.
// After selecting an entity, WASD will instead move that entity along the world-axes.
// To move the entity relative to the view, press M
// To rotate or scale an entity, press R and T respectively, then use WASD.
// Holding Shift speeds up all WASD-based inputs.
// Holding Ctrl slows down all WASD-based inputs.
// Pressing P randomly spawns 25 entities across the map, possibly outside of the starting room.
// Pressing O spawns one entity in front of the camera, with a model and texture that can be selected beforehand
// I think this is done by holding M/T while pressing +/-. I don't remember exactly and am too lazy to check.
// Pressing G enables light rotation.
// Pressing F enables point-light movement.
// Pressing C cycles between the main camera, all cubemap cameras and all spotlight cameras.
// Pressing Z enables moving the current camera instead of the main camera.
// Pressing Q resets all/most toggles.

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

	constexpr UINT width = 2560;
	constexpr UINT height = 1440;
	//constexpr UINT width = 1920;
	//constexpr UINT height = 1080;
	HWND window;

	if (!SetupWindow(hInstance, width, height, nCmdShow, window))
	{
		ErrMsg("Failed to setup window!");
		return -1;
	}

	Time time = { };
	Input input = { };
	MSG msg = { };

	Game game = { };
	if (!game.Setup(time, width, height, window))
	{
		ErrMsg("Failed to setup graphics!");
		return -1;
	}

	// Print asset load times.
	ErrMsg(std::format("Mesh Load: {} s", time.CompareSnapshots("LoadMeshes")));
	ErrMsg(std::format("Texture Load: {} s", time.CompareSnapshots("LoadTextures")));
	ErrMsg(std::format("Texture Map Load: {} s", time.CompareSnapshots("LoadTextureMaps")));
	ErrMsg(std::format("Shader Load: {} s", time.CompareSnapshots("LoadShaders")));

	Scene scene = { };
	if (!game.SetScene(time, &scene))
	{
		ErrMsg("Failed to set scene!");
		return -1;
	}
	
	// Main game loop.
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

		if (!game.Render(time, input))
		{
			ErrMsg("Failed to render frame!");
			return -1;
		}
	}

	OutputDebugString(L"========| End |============================================================================\n\n");
	return 0;
}
