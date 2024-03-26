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


	struct TextureData { std::string name; std::string file; };
	const std::vector<TextureData> textureNames = {
		// Opaque
		{ "Error",							"Error"							},
		{ "Fallback",						"Fallback"						},
		{ "texture1",						"texture1"						},
		{ "texture2",						"texture2"						},
		{ "texture3",						"texture3"						},
		{ "texture4",						"texture4"						},
		{ "texture5",						"texture5"						},
		{ "CharacterSculptLow0Texture1",	"CharacterSculptLow0Texture1"	},
		{ "Sphere",							"Sphere"						},
		{ "White",							"White"							},
		{ "Black",							"Black"							},
		{ "Fade",							"Fade"							},
		{ "Bricks",							"Bricks"						},
		{ "Metal",							"Metal"							},

		//Transparent
		{ "Transparent",					"Transparent"					},
		{ "Transparent2",					"Transparent2"					},
		{ "Particle",						"Particle"						},
	};

	for (const TextureData &textureName : textureNames)
		if (_content.AddTexture(_device, std::format("Tex_{}", textureName.name), 
			std::format("Content\\{}.png", textureName.file).c_str()) == CONTENT_LOAD_ERROR)
		{
			ErrMsg(std::format("Failed to add Tex_{}!", textureName.name));
			return false;
		}


	struct TextureMapData { TextureType type; std::string name; std::string file; };
	const std::vector<TextureMapData> textureMapNames = {
		{ TextureType::NORMAL,		"Default_Normal",				"Default_Normal"				},
		{ TextureType::NORMAL,		"texture3_Normal",				"texture3_Normal"				},
		{ TextureType::NORMAL,		"Bricks_Normal",				"Bricks_Normal"					},
		{ TextureType::NORMAL,		"Shapes_Normal",				"Shapes_Normal"					},
		{ TextureType::NORMAL,		"Metal_Normal",					"Metal_Normal"					},

		{ TextureType::SPECULAR,	"Default_Specular",				"Black"							},
		{ TextureType::SPECULAR,	"Gray_Specular",				"Gray"							},
		{ TextureType::SPECULAR,	"White_Specular",				"White"							},
		{ TextureType::SPECULAR,	"Fade_Specular",				"Fade"							},
		{ TextureType::SPECULAR,	"CharacterSculptLow0_Specular",	"CharacterSculptLow0_Specular"	},
		{ TextureType::SPECULAR,	"Bricks_Specular",				"Bricks_Specular"				},
		{ TextureType::SPECULAR,	"Metal_Specular",				"Metal_Specular"				},

		{ TextureType::REFLECTIVE,	"Default_Reflective",			"Black"							},
		{ TextureType::REFLECTIVE,	"Gray_Reflective",				"Gray"							},
		{ TextureType::REFLECTIVE,	"White_Reflective",				"White"							},
		{ TextureType::REFLECTIVE,	"Fade_Reflective",				"Fade"							},
		{ TextureType::REFLECTIVE,	"Metal_Reflective",				"Metal"							},

		{ TextureType::HEIGHT,		"Default_Height",				"Gray"							},
		{ TextureType::HEIGHT,		"Metal_Height",					"Metal_Height"					},
	};

	for (const TextureMapData &textureMap : textureMapNames)
		if (_content.AddTextureMap(_device, std::format("TexMap_{}", textureMap.name), 
			textureMap.type, std::format("Content\\{}.png", textureMap.file).c_str()) == CONTENT_LOAD_ERROR)
		{
			ErrMsg(std::format("Failed to add TexMap_{}!", textureMap.name));
			return false;
		}


	struct ShaderData { ShaderType type; std::string name; std::string file; };
	const std::vector<ShaderData> shaderNames = {
		{ ShaderType::VERTEX_SHADER,		"VS_Geometry",			"VS_Geometry"			},
		{ ShaderType::VERTEX_SHADER,		"VS_Depth",				"VS_Depth"				},
		{ ShaderType::VERTEX_SHADER,		"VS_Particle",			"VS_Particle"			},
		{ ShaderType::HULL_SHADER,			"HS_LOD",				"HS_LOD"				},
		{ ShaderType::DOMAIN_SHADER,		"DS_LOD",				"DS_LOD"				},
		{ ShaderType::GEOMETRY_SHADER,		"GS_Billboard",			"GS_Billboard"			},
		{ ShaderType::PIXEL_SHADER,			"PS_Geometry",			"PS_Geometry"			},
		{ ShaderType::PIXEL_SHADER,			"PS_Transparent",		"PS_Transparent"		},
		{ ShaderType::PIXEL_SHADER,			"PS_Particle",			"PS_Particle"			},
		{ ShaderType::COMPUTE_SHADER,		"CS_Lighting",			"CS_Lighting"			},
		{ ShaderType::COMPUTE_SHADER,		"CS_CubemapLighting",	"CS_CubemapLighting"	},
		{ ShaderType::COMPUTE_SHADER,		"CS_GBuffer",			"CS_GBuffer"			},
		{ ShaderType::COMPUTE_SHADER,		"CS_Particle",			"CS_Particle"			},
	};

	for (const ShaderData &shader : shaderNames)
		if (_content.AddShader(_device, shader.name, shader.type, std::format("Content\\{}.cso", shader.file).c_str()) == CONTENT_LOAD_ERROR)
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

	if (_graphics.GetUpdateCubemap())
	{
		snprintf(timeStr, sizeof(timeStr), "%.6f", time.CompareSnapshots("FrustumCullCubemap"));
		ImGui::Text(std::format("{} Culling Cubemap Views", timeStr).c_str());
	}

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