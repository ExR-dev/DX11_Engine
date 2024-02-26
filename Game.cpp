#include "Game.h"

#include "ErrMsg.h"


Game::Game()
{
	_device				= nullptr;
	_immediateContext	= nullptr;

	//_graphics	= { };

	_scene		= nullptr;
}

Game::~Game()
{
	if (_immediateContext != nullptr)
		_immediateContext->Release();

	if (_device != nullptr)
		_device->Release();
}

bool Game::Setup(const UINT width, const UINT height, const HWND window)
{
	if (!_graphics.Setup(width, height, window, _device, _immediateContext, &_content))
	{
		ErrMsg("Failed to setup d3d11!");
		return false;
	}


	if (_content.AddMesh(_device, "FallbackMesh", "Content\\Fallback.obj") == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add fallback mesh!");
		return false;
	}

	if (_content.AddMesh(_device, "ShapeMesh", "Content\\ShapeTri.obj") == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add shape mesh!");
		return false;
	}

	if (_content.AddMesh(_device, "SimpleSubMesh", "Content\\SimpleSubmesh.obj") == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add simpleSubmesh mesh!");
		return false;
	}

	if (_content.AddMesh(_device, "ControlChairMesh", "Content\\ControlChair.obj") == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add controlChair mesh!");
		return false;
	}

	if (_content.AddMesh(_device, "ControlDeskMesh", "Content\\ControlDesk.obj") == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add controlDesk mesh!");
		return false;
	}

	if (_content.AddMesh(_device, "RoomCorridorMesh", "Content\\RoomCorridor.obj") == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add roomCorridor mesh!");
		return false;
	}

	if (_content.AddMesh(_device, "CharacterSculptMesh", "Content\\CharacterSculpt.obj") == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add characterSculpt mesh!");
		return false;
	}

	if (_content.AddMesh(_device, "CharacterSculptLow1Mesh", "Content\\CharacterSculptLow1.obj") == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add characterSculptLow1 mesh!");
		return false;
	}

	if (_content.AddMesh(_device, "CharacterSculptLow2Mesh", "Content\\CharacterSculptLow2.obj") == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add characterSculptLow2 mesh!");
		return false;
	}


	const UINT fallbackVShaderID = _content.AddShader(_device, "FallbackVShader", ShaderType::VERTEX_SHADER, "Content\\VertexShader.cso");
	if (fallbackVShaderID == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add fallback vertex shader!");
		return false;
	}

	if (_content.AddShader(_device, "FallbackPShader", ShaderType::PIXEL_SHADER, "Content\\PixelShader.cso") == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add fallback pixel shader!");
		return false;
	}


	if (_content.AddTexture(_device, "FallbackTexture", "Content\\texture1.png") == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add fallback texture!");
		return false;
	}

	if (_content.AddTexture(_device, "Texture2", "Content\\texture2.png") == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add texture2!");
		return false;
	}

	if (_content.AddTexture(_device, "Texture3", "Content\\texture3.png") == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add texture3!");
		return false;
	}

	if (_content.AddTexture(_device, "Texture4", "Content\\texture4.png") == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add texture4!");
		return false;
	}

	if (_content.AddTexture(_device, "Texture5", "Content\\texture5.png") == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add texture5!");
		return false;
	}

	if (_content.AddTexture(_device, "Texture6", "Content\\texture6.png") == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add texture6!");
		return false;
	}

	if (_content.AddTexture(_device, "CharacterSculptTexture", "Content\\CharacterSculptTexture.png") == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add characterSculptTexture!");
		return false;
	}

	if (_content.AddTexture(_device, "CharacterSculptLow1Texture", "Content\\CharacterSculptLow1Texture.png") == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add characterSculptLow1Texture!");
		return false;
	}

	if (_content.AddTexture(_device, "CharacterSculptLow2Texture", "Content\\CharacterSculptLow2Texture.png") == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add characterSculptLow2Texture!");
		return false;
	}


	if (_content.AddSampler(_device, "FallbackSampler", D3D11_TEXTURE_ADDRESS_BORDER) == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add fallback sampler!");
		return false;
	}


	const std::vector<Semantic> fallbackInputLayout{
		{ "POSITION",	DXGI_FORMAT_R32G32B32_FLOAT },
		{ "NORMAL",		DXGI_FORMAT_R32G32B32_FLOAT },
		{ "TEXCOORD",	DXGI_FORMAT_R32G32_FLOAT	}
	};

	if (_content.AddInputLayout(_device, "FallbackInputLayout", fallbackInputLayout, fallbackVShaderID) == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add fallback input layout!");
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
		ErrMsg("Failed to initialize scene!");
		return false;
	}

	return true;
}


bool Game::Update(const Time &time, const Input &input)
{
	/// v==========================================v ///
	/// v        Update game logic here...         v ///
	/// v==========================================v ///

	if (_scene != nullptr)
		if (!_scene->Update(_immediateContext, time, input))
		{
			ErrMsg("Failed to update scene!");
			return false;
		}

	/// ^==========================================^ ///
	/// ^        Update game logic here...         ^ ///
	/// ^==========================================^ ///

	return true;
}


bool Game::Render(const Time &time, const Input &input)
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
		if (!_scene->Render(&_graphics, time, input))
		{
			ErrMsg("Failed to render scene!");
			return false;
		}

	/// ^==========================================^ ///
	/// ^        Render scene here...              ^ ///
	/// ^==========================================^ ///

	if (!_graphics.EndRender(time))
	{
		ErrMsg("Failed to end rendering!\n");
		return false;
	}

	return true;
}