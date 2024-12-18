#pragma once

#include <Windows.h>


[[nodiscard]] bool SetupWindow(
	HINSTANCE instance, 
	UINT width, UINT height, 
	int nCmdShow, HWND& window
);