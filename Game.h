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

	[[nodiscard]] bool Setup(Time &time, UINT width, UINT height, HWND window);
	[[nodiscard]] bool SetScene(Time &time, Scene *scene);

	[[nodiscard]] bool Update(Time &time, const Input &input) const;
	[[nodiscard]] bool Render(Time &time, const Input &input);
};
