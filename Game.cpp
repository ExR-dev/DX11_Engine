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

bool Game::SetupGraphics(UINT width, UINT height, HWND window, DebugData &debugData)
{
	if (!_graphics.Setup(width, height, window, _device, _immediateContext))
	{
		std::cerr << "Failed to setup d3d11!" << std::endl;
		return false;
	}

	if (!debugData.Initialize(_device))
	{
		std::cerr << "Failed to initialize debug data!" << std::endl;
		return false;
	}

	return true;
}

bool Game::SetScene(Scene *scene)
{
	_scene = scene;
	return true;
}


bool Game::Update(const Time &time, DebugData &debugData)
{
	/// Update game logic here...

	//XMVECTOR camPos = { debugData.fallback.lightingBufferData.camPos[0], debugData.fallback.lightingBufferData.camPos[1], debugData.fallback.lightingBufferData.camPos[2] };
	XMVECTOR camPos = { 0, 0, 0 };
	XMVECTOR camDir = { 0, 0, 1 };

	if (!debugData.Update(_device, _immediateContext, 1, 1, time, camPos, camDir))
	{
		std::cerr << "Failed to update debug data!" << std::endl;
		return false;
	}

	/// Update game logic here...

	return true;
}

bool Game::Render(const Time &time, DebugData &debugData)
{
	UINT vertexCount = 0;

	if (!_graphics.BeginRender(_immediateContext, debugData))
	{
		std::cerr << "Failed to begin rendering!" << std::endl;
		return false;
	}

	/// Render scene here...

	if (!debugData.Draw(_immediateContext, vertexCount, _graphics.GetRTV(), _graphics.GetDsView(), _graphics.GetViewport()))
	{
		std::cerr << "Failed to draw debug data!" << std::endl;
		return false;
	}
	
	/// Render scene here...

	if (!_graphics.EndRender(_immediateContext, vertexCount))
	{
		std::cerr << "Failed to end rendering!" << std::endl;
		return false;
	}
	return true;
}