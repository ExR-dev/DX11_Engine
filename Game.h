#pragma once

#include <iostream>
#include <d3d11.h>
#include <DirectXMath.h>

#include "Data.h"
#include "Scene.h"
#include "Graphics.h"
#include "Time.h"
#include "D3D11Helper.h"
#include "PipelineHelper.h"

using namespace DirectX;


class Game
{
private:
	Scene _scene = Scene();
	Graphics _graphics = Graphics();

public:
	Game();
	~Game();

	int Update(const Data &data, const Time &time);
	int Render(const Data &data, const Time &time);
};
