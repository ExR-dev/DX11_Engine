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

bool Game::Setup(UINT width, UINT height, HWND window)
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
	if (scene == nullptr)
		return false;

	_scene = scene;

	if (!_scene->Initialize(_device))
	{
		std::cerr << "Failed to initialize scene!" << std::endl;
		return false;
	}

	return true;
}


bool Game::Update(const Time &time)
{
	/// v==========================================v ///
	/// v        Update game logic here...         v ///
	/// v==========================================v ///

	if (_scene != nullptr)
		if (!_scene->Update(_immediateContext, time))
		{
			std::cerr << "Failed to update scene!" << std::endl;
			return false;
		}

	/// ^==========================================^ ///
	/// ^        Update game logic here...         ^ ///
	/// ^==========================================^ ///

	return true;
}


bool Game::Render(const Time &time)
{
	if (!_graphics.BeginRender())
	{
		std::cerr << "Failed to begin rendering!" << std::endl;
		return false;
	}

	/// v==========================================v ///
	/// v        Render scene here...              v ///
	/// v==========================================v ///

	if (_scene != nullptr)
		if (!_scene->Render(_immediateContext))
		{
			std::cerr << "Failed to render scene!" << std::endl;
			return false;
		}


	/// ^==========================================^ ///
	/// ^        Render scene here...              ^ ///
	/// ^==========================================^ ///

	if (!_graphics.EndRender())
	{
		std::cerr << "Failed to end rendering!" << std::endl;
		return false;
	}

	return true;
}