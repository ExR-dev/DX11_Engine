#pragma once

#include <d3d11.h>

#include "Scene.h"
#include "Graphics.h"
#include "Time.h"


class Game
{
private:
	ID3D11Device		*_device;
	ID3D11DeviceContext *_immediateContext;

	Graphics	_graphics;
	Scene		*_scene;

public:
	Game();
	~Game();

	bool Setup(UINT width, UINT height, HWND window);

	bool SetScene(Scene *scene);

	bool Update(const Time &time);
	bool Render(const Time &time);
};
