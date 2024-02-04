#pragma once

#include <iostream>
#include <d3d11.h>
#include <DirectXMath.h>

#include "Data.h"
#include "Time.h"
#include "D3D11Helper.h"
#include "PipelineHelper.h"

using namespace DirectX;


class Game
{
private:

public:
	Game();
	~Game();

	int Update(Data *data, Time *time);

	int Render(Data *data, Time *time);
};


inline Game::Game()
{ }

inline Game::~Game()
{ }

inline int Game::Update(Data *data, Time *time)
{

	return 0;
}

inline int Game::Render(Data *data, Time *time)
{

	return 0;
}
