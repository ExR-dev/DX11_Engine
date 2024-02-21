#include "Game.h"

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

bool Game::Setup(const UINT width, const UINT height, const HWND window)
{
	if (!_graphics.Setup(width, height, window, _device, _immediateContext, &_content))
	{
		ErrMsg("Failed to setup d3d11!");
		return false;
	}


	const UINT fallbackMeshID = _content.AddMesh(_device, "FallbackMesh", "Content\\Fallback.obj");
	if (fallbackMeshID == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add fallback mesh!");
		return false;
	}

	const UINT shapeMeshID = _content.AddMesh(_device, "ShapeMesh", "Content\\ShapeTri.obj");
	if (shapeMeshID == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add shape mesh!");
		return false;
	}

	const UINT simpleSubMeshID = _content.AddMesh(_device, "SimpleSubMesh", "Content\\SimpleSubmesh.obj");
	if (simpleSubMeshID == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add simpleSubmesh mesh!");
		return false;
	}


	const UINT fallbackVShaderID = _content.AddShader(_device, "FallbackVShader", ShaderType::VERTEX_SHADER, "Content\\VertexShader.cso");
	if (fallbackVShaderID == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add fallback vertex shader!");
		return false;
	}

	const UINT fallbackPShaderID = _content.AddShader(_device, "FallbackPShader", ShaderType::PIXEL_SHADER, "Content\\PixelShader.cso");
	if (fallbackPShaderID == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add fallback pixel shader!");
		return false;
	}


	const UINT fallbackTextureID = _content.AddTexture(_device, "FallbackTexture", "Content\\texture.png");
	if (fallbackTextureID == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add fallback texture!");
		return false;
	}


	const UINT fallbackSamplerID = _content.AddSampler(_device, "FallbackSampler", D3D11_TEXTURE_ADDRESS_BORDER);
	if (fallbackSamplerID == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add fallback sampler!");
		return false;
	}


	const std::vector<Semantic> fallbackInputLayout{
		{ "POSITION",	DXGI_FORMAT_R32G32B32_FLOAT },
		{ "NORMAL",		DXGI_FORMAT_R32G32B32_FLOAT },
		{ "TEXCOORD",	DXGI_FORMAT_R32G32_FLOAT	}
	};

	const UINT fallbackInputLayoutID = _content.AddInputLayout(_device, "FallbackInputLayout", fallbackInputLayout, fallbackVShaderID);
	if (fallbackInputLayoutID == CONTENT_LOAD_ERROR)
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
		if (!_scene->Render(&_graphics))
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