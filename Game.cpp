#include "Game.h"

#include "ErrMsg.h"
#include "ImGui/imgui.h"


Game::Game()
{
	_device				= nullptr;
	_immediateContext	= nullptr;
	_scene				= nullptr;
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

	// TODO: Add support for reading .mtl files.

	const std::vector<std::string> meshNames = {
		"Error",
		"Fallback",
		"Cube",
		"Room",
		"SimpleSubmesh",
		"ControlChair",
		"ControlDesk",
		"CharacterSculptLow0",
		"CharacterSculptLow1",
		"Plane",
		"Sphere",
		"IsoSphereEdged",
		"IsoSphereSmooth",
	};

	for (const std::string &meshName : meshNames)
		if (_content.AddMesh(_device, std::format("Mesh_{}", meshName), std::format("Content\\{}.obj", meshName).c_str()) == CONTENT_LOAD_ERROR)
		{
			ErrMsg(std::format("Failed to add Mesh_{}!", meshName));
			return false;
		}


	const std::vector<std::string> textureNames = {
		// Opaque
		"Error",
		"Fallback",
		"texture1",
		"texture2",
		"texture3",
		"texture4",
		"texture5",
		"CharacterSculptLow0Texture1",
		"Sphere",
		"Default",
		"Black",
		"Fade",
		"Bricks",

		//Transparent
		"Transparent",
		"Transparent2",
		"Particle",
	};

	for (const std::string &textureName : textureNames)
		if (_content.AddTexture(_device, std::format("Tex_{}", textureName), 
			std::format("Content\\{}.png", textureName).c_str()) == CONTENT_LOAD_ERROR)
		{
			ErrMsg(std::format("Failed to add Tex_{}!", textureName));
			return false;
		}


	struct TextureMapData { TextureType type; std::string name; };
	const std::vector<TextureMapData> textureMapNames = {
		{ TextureType::NORMAL,		"Default_Normal" },
		{ TextureType::NORMAL,		"texture3_Normal" },
		{ TextureType::NORMAL,		"Bricks_Normal" },
		{ TextureType::SPECULAR,	"Default_Specular" },
		{ TextureType::SPECULAR,	"CharacterSculptLow0_Specular" },
		{ TextureType::SPECULAR,	"Black" },
		{ TextureType::SPECULAR,	"Fade" },
		{ TextureType::SPECULAR,	"Bricks_Specular" },
	};

	for (const TextureMapData &textureMap : textureMapNames)
		if (_content.AddTextureMap(_device, std::format("TexMap_{}", textureMap.name), 
			textureMap.type, std::format("Content\\{}.png", textureMap.name).c_str()) == CONTENT_LOAD_ERROR)
		{
			ErrMsg(std::format("Failed to add TexMap_{}!", textureMap.name));
			return false;
		}


	struct ShaderData { ShaderType type; std::string name; };
	const std::vector<ShaderData> shaderNames = {
		{ ShaderType::VERTEX_SHADER,		"VS_Geometry" },
		{ ShaderType::VERTEX_SHADER,		"VS_Depth" },
		{ ShaderType::VERTEX_SHADER,		"VS_Particle" },
		{ ShaderType::GEOMETRY_SHADER,		"GS_Billboard" },
		{ ShaderType::PIXEL_SHADER,			"PS_Geometry" },
		{ ShaderType::PIXEL_SHADER,			"PS_Transparent" },
		{ ShaderType::PIXEL_SHADER,			"PS_Particle" },
		{ ShaderType::COMPUTE_SHADER,		"CS_Lighting" },
		{ ShaderType::COMPUTE_SHADER,		"CS_GBuffer" },
		{ ShaderType::COMPUTE_SHADER,		"CS_Particle" },
	};

	for (const ShaderData &shader : shaderNames)
		if (_content.AddShader(_device, shader.name, shader.type, std::format("Content\\{}.cso", shader.name).c_str()) == CONTENT_LOAD_ERROR)
		{
			ErrMsg(std::format("Failed to add {} shader!", shader.name));
			return false;
		}


	if (_content.AddSampler(_device, "SS_Fallback", D3D11_TEXTURE_ADDRESS_BORDER) == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add fallback sampler!");
		return false;
	}


	const std::vector<Semantic> fallbackInputLayout {
		{ "POSITION",	DXGI_FORMAT_R32G32B32_FLOAT },
		{ "NORMAL",		DXGI_FORMAT_R32G32B32_FLOAT },
		{ "TANGENT",	DXGI_FORMAT_R32G32B32_FLOAT },
		{ "TEXCOORD",	DXGI_FORMAT_R32G32_FLOAT	}
	};

	if (_content.AddInputLayout(_device, "IL_Fallback", fallbackInputLayout, _content.GetShaderID("VS_Geometry")) == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to add IL_Fallback!");
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


bool Game::Update(Time &time, const Input &input)
{
	/// v==========================================v ///
	/// v        Update game logic here...         v ///
	/// v==========================================v ///

	time.TakeSnapshot("SceneUpdateTime");
	if (_scene != nullptr)
		if (!_scene->Update(_immediateContext, time, input))
		{
			ErrMsg("Failed to update scene!");
			return false;
		}
	time.TakeSnapshot("SceneUpdateTime");

	/// ^==========================================^ ///
	/// ^        Update game logic here...         ^ ///
	/// ^==========================================^ ///

	return true;
}


bool Game::Render(Time &time, const Input &input)
{
	if (!_graphics.BeginSceneRender())
	{
		ErrMsg("Failed to begin rendering!");
		return false;
	}

	/// v==========================================v ///
	/// v        Render scene here...              v ///
	/// v==========================================v ///

	time.TakeSnapshot("SceneRenderTime");
	if (_scene != nullptr)
		if (!_scene->Render(&_graphics, time, input))
		{
			ErrMsg("Failed to render scene!");
			return false;
		}
	time.TakeSnapshot("SceneRenderTime");

	/// ^==========================================^ ///
	/// ^        Render scene here...              ^ ///
	/// ^==========================================^ ///

	if (!_graphics.EndSceneRender(time))
	{
		ErrMsg("Failed to end rendering!");
		return false;
	}

	
//#ifdef _DEBUG
	if (!_graphics.BeginUIRender())
	{
		ErrMsg("Failed to begin UI rendering!");
		return false;
	}

	/// v==========================================v ///
	/// v        Render UI here...                 v ///
	/// v==========================================v ///

	if (!_graphics.RenderUI(time))
	{
		ErrMsg("Failed to render graphics UI!");
		return false;
	}

	ImGui::Separator();

	if (_scene != nullptr)
		if (!_scene->RenderUI())
		{
			ErrMsg("Failed to render scene UI!");
			return false;
		}

	ImGui::Separator();

	char timeStr[32]{};

	snprintf(timeStr, sizeof(timeStr), "%.6f", time.deltaTime);
	ImGui::Text(std::format("{} Frame", timeStr).c_str());

	snprintf(timeStr, sizeof(timeStr), "%.6f", time.CompareSnapshots("SceneUpdateTime"));
	ImGui::Text(std::format("{} Scene Update", timeStr).c_str());

	snprintf(timeStr, sizeof(timeStr), "%.6f", time.CompareSnapshots("SceneRenderTime"));
	ImGui::Text(std::format("{} Scene Render", timeStr).c_str());

	snprintf(timeStr, sizeof(timeStr), "%.6f", time.CompareSnapshots("FrustumCull"));
	ImGui::Text(std::format("{} Culling Main View", timeStr).c_str());

	snprintf(timeStr, sizeof(timeStr), "%.6f", time.CompareSnapshots("FrustumCullSpotlights"));
	ImGui::Text(std::format("{} Culling Spotlights Total", timeStr).c_str());

	int i = 0;
	float spotlightTime = time.CompareSnapshots(std::format("FrustumCullSpotlight{}", i));
	while (spotlightTime >= 0.0f)
	{
		snprintf(timeStr, sizeof(timeStr), "%.6f", spotlightTime);
		ImGui::Text(std::format("{} Culling Spotlight #{}", timeStr, i).c_str());

		spotlightTime = time.CompareSnapshots(std::format("FrustumCullSpotlight{}", ++i));
	}

	/// ^==========================================^ ///
	/// ^        Render UI here...                 ^ ///
	/// ^==========================================^ ///

	if (!_graphics.EndUIRender())
	{
		ErrMsg("Failed to end UI rendering!");
		return false;
	}
//#endif // _DEBUG


	if (!_graphics.EndFrame())
	{
		ErrMsg("Failed to end frame!");
		return false;
	}

	return true;
}