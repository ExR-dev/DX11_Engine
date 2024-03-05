#pragma once

#include <d3d11.h>

#include "Scene.h"
#include "Graphics.h"
#include "Content.h"
#include "Time.h"
#include "Input.h"


class Game
{
private:
	ID3D11Device		*_device;
	ID3D11DeviceContext *_immediateContext;

	Graphics	_graphics;
	Content		_content;
	Scene		*_scene;


public:
	Game();
	~Game();

	[[nodiscard]] bool Setup(UINT width, UINT height, HWND window);
	[[nodiscard]] bool SetScene(Scene *scene);

	[[nodiscard]] bool Update(Time &time, const Input &input);
	[[nodiscard]] bool Render(Time &time, const Input &input);
};
