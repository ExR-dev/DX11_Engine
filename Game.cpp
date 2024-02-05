#include "Game.h"

#include <iostream>


Game::Game()
{
	_device				= nullptr;
	_immediateContext	= nullptr;

	_graphics	= { };
	_scene		= nullptr;
}

Game::~Game()
{
	if (_immediateContext != nullptr)
		_immediateContext->Release();

	if (_device != nullptr)
		_device->Release();
}

bool Game::SetupGraphics(UINT width, UINT height, HWND window)
{
	if (!_graphics.Setup(width, height, window, _device, _immediateContext))
	{
		std::cerr << "Failed to setup d3d11!" << std::endl;
		return false;
	}
	return true;
}

bool Game::SetScene(Scene *scene)
{
	_scene = scene;
	return true;
}


bool Game::Update(const Time &time)
{
	// Update game logic here...

	return true;
}

bool Game::Render(const Time &time)
{
	if (!_graphics.BeginRender())
	{
		std::cerr << "Failed to begin rendering!" << std::endl;
		return false;
	}

	// Render scene here..

	if (!_graphics.EndRender())
	{
		std::cerr << "Failed to end rendering!" << std::endl;
		return false;
	}
	return true;
}