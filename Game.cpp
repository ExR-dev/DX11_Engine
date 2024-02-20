#include "Game.h"

#include <iostream>

#include "ErrMsg.h"


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
		ErrMsg("Failed to setup d3d11!");
		return false;
	}

	const UINT fallbackMeshID = _content.AddMesh(_device, "Fallback", "Models\\Fallback.obj");
	if (fallbackMeshID == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add fallback mesh!");
		return false;
	}

	const UINT ShapeMeshID = _content.AddMesh(_device, "Shape", "Models\\ShapeTri.obj");
	if (ShapeMeshID == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add shape mesh!");
		return false;
	}

	const UINT SubMeshID = _content.AddMesh(_device, "Submesh", "Models\\SimpleSubmesh.obj");
	if (SubMeshID == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add simpleSubmesh mesh!");
		return false;
	}

	return true;
}

bool Game::SetScene(Scene *scene)
{
	if (scene == nullptr)
		return false;

	_scene = scene;

	if (!_scene->Initialize(_device, &_content))
	{
		ErrMsg("Failed to initialize scene!");
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
			ErrMsg("Failed to update scene!");
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
		ErrMsg("Failed to begin rendering!");
		return false;
	}

	/// v==========================================v ///
	/// v        Render scene here...              v ///
	/// v==========================================v ///

	if (_scene != nullptr)
		if (!_scene->Render(_immediateContext, _content))
		{
			ErrMsg("Failed to render scene!");
			return false;
		}

	/// ^==========================================^ ///
	/// ^        Render scene here...              ^ ///
	/// ^==========================================^ ///

	if (!_graphics.EndRender())
	{
		ErrMsg("Failed to end rendering!\n");
		return false;
	}

	return true;
}