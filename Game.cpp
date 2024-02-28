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


	const std::vector<std::string> meshNames = {
		"Error",
		"Fallback",
		"ShapeTri",
		"SimpleSubmesh",
		"ControlChair",
		"ControlDesk",
		"CharacterSculptLow0",
		"CharacterSculptLow1",
	};

	for (const std::string &meshName : meshNames)
		if (_content.AddMesh(_device, std::format("Mesh_{}", meshName), std::format("Content\\{}.obj", meshName).c_str()) == CONTENT_LOAD_ERROR)
		{
			ErrMsg(std::format("Failed to add Mesh_{}!", meshName));
			return false;
		}


	const std::vector<std::string> textureNames = {
		"Error",
		"Fallback",
		"texture1",
		"texture2",
		"texture3",
		"texture4",
		"texture5",
		"CharacterSculptLow0Texture",
		"CharacterSculptLow0Texture1",
		"CharacterSculptLow1Texture",
	};

	for (const std::string &textureName : textureNames)
		if (_content.AddTexture(_device, std::format("Tex_{}", textureName), std::format("Content\\{}.png", textureName).c_str()) == CONTENT_LOAD_ERROR)
		{
			ErrMsg(std::format("Failed to add Tex_{}!", textureName));
			return false;
		}


	const UINT geometryVShaderID = _content.AddShader(_device, "VS_Geometry", ShaderType::VERTEX_SHADER, "Content\\VS_Geometry.cso");
	if (geometryVShaderID == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add VS_Geometry shader!");
		return false;
	}

	const UINT depthVShaderID = _content.AddShader(_device, "VS_Depth", ShaderType::VERTEX_SHADER, "Content\\VS_Depth.cso");
	if (geometryVShaderID == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add VS_Depth shader!");
		return false;
	}

	if (_content.AddShader(_device, "PS_Geometry", ShaderType::PIXEL_SHADER, "Content\\PS_Geometry.cso") == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add PS_Geometry shader!");
		return false;
	}

	if (_content.AddShader(_device, "CS_Lighting", ShaderType::COMPUTE_SHADER, "Content\\CS_Lighting.cso") == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add CS_Lighting shader!");
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

	if (_content.AddInputLayout(_device, "IL_Fallback", fallbackInputLayout, geometryVShaderID) == CONTENT_LOAD_ERROR)
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