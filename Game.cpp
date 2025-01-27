#include "Game.h"

#include "ErrMsg.h"
#include "ImGui/imgui.h"


Game::Game()
{
	_device = nullptr;
	_immediateContext = nullptr;
	_scene = nullptr;
}

Game::~Game()
{
	if (_immediateContext != nullptr)
		_immediateContext->Release();

	if (_device != nullptr)
		_device->Release();
}

bool Game::LoadContent(Time& time,
	const std::vector<std::string>& meshNames,
	const std::vector<TextureData>& textureNames,
	const std::vector<TextureMapData>& textureMapNames,
	const std::vector<ShaderData>& shaderNames)
{
	time.TakeSnapshot("LoadMeshes");
	for (const std::string& meshName : meshNames)
		if (_content.AddMesh(_device, std::format("Mesh_{}", meshName), std::format("Content\\Meshes\\{}.obj", meshName).c_str()) == CONTENT_NULL)
		{
			ErrMsg(std::format("Failed to add Mesh_{}!", meshName));
			return false;
		}
	time.TakeSnapshot("LoadMeshes");


	time.TakeSnapshot("LoadTextures");
	for (const TextureData& textureName : textureNames)
		if (_content.AddTexture(_device, _immediateContext, std::format("Tex_{}", textureName.name),
			std::format("Content\\Textures\\{}.png", textureName.file).c_str()) == CONTENT_NULL)
		{
			ErrMsg(std::format("Failed to add Tex_{}!", textureName.name));
			return false;
		}
	time.TakeSnapshot("LoadTextures");


	time.TakeSnapshot("LoadTextureMaps");
	for (const TextureMapData& textureMap : textureMapNames)
		if (_content.AddTextureMap(_device, _immediateContext, std::format("TexMap_{}", textureMap.name),
			textureMap.type, std::format("Content\\Textures\\{}.png", textureMap.file).c_str()) == CONTENT_NULL)
		{
			ErrMsg(std::format("Failed to add TexMap_{}!", textureMap.name));
			return false;
		}
	time.TakeSnapshot("LoadTextureMaps");


	time.TakeSnapshot("LoadShaders");
	for (const ShaderData& shader : shaderNames)
		if (_content.AddShader(_device, shader.name, shader.type, std::format("Content\\Shaders\\{}.cso", shader.file).c_str()) == CONTENT_NULL)
		{
			ErrMsg(std::format("Failed to add {} shader!", shader.name));
			return false;
		}
	time.TakeSnapshot("LoadShaders");


	if (_content.AddSampler(_device, "SS_Fallback", D3D11_TEXTURE_ADDRESS_BORDER) == CONTENT_NULL)
	{
		ErrMsg("Failed to add fallback sampler!");
		return false;
	}

	if (_content.AddSampler(_device, "SS_Clamp", D3D11_TEXTURE_ADDRESS_CLAMP) == CONTENT_NULL)
	{
		ErrMsg("Failed to add clamp sampler!");
		return false;
	}

	if (_content.AddSampler(_device, "SS_Shadow", D3D11_TEXTURE_ADDRESS_CLAMP, std::nullopt, false) == CONTENT_NULL)
	{
		ErrMsg("Failed to add shadow sampler!");
		return false;
	}


	const std::vector<Semantic> fallbackInputLayout{
		{ "POSITION",	DXGI_FORMAT_R32G32B32_FLOAT },
		{ "NORMAL",		DXGI_FORMAT_R32G32B32_FLOAT },
		{ "TANGENT",	DXGI_FORMAT_R32G32B32_FLOAT },
		{ "TEXCOORD",	DXGI_FORMAT_R32G32_FLOAT	}
	};

	if (_content.AddInputLayout(_device, "IL_Fallback", fallbackInputLayout, _content.GetShaderID("VS_Geometry")) == CONTENT_NULL)
	{
		ErrMsg("Failed to add IL_Fallback!");
		return false;
	}

	return true;
}


bool Game::Setup(Time& time, const UINT width, const UINT height, const HWND window)
{
	if (!_graphics.Setup(width, height, window, _device, _immediateContext, &_content))
	{
		ErrMsg("Failed to setup d3d11!");
		return false;
	}

	const std::vector<std::string> meshNames = {
		"Error",
		"Fallback",
		"Cube",
		"Room",
		"SimpleSubmesh",
		"CharacterSculptLow1",
		"Plane",
		"Sphere",
		"IsoSphereSmooth",
		"Torus",
		"WireframeCube",
		"TransformGizmo",
	};

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
		{ "Ambient",						"Ambient"						},
		{ "Red",							"Red"							},
		{ "Green",							"Green"							},
		{ "Blue",							"Blue"							},
		{ "Fade",							"Fade"							},
		{ "Bricks",							"Bricks"						},
		{ "Metal",							"Metal"							},
		{ "Cobble",							"Cobble"						},
		{ "TransformGizmo",					"TransformGizmo"				},

		//Transparent
		{ "Transparent",					"Transparent"					},
		{ "Flower",							"Transparent2"					},
		{ "Particle",						"Particle"						},
	};

	const std::vector<TextureMapData> textureMapNames = {
		{ TextureType::NORMAL,		"Default_Normal",				"Default_Normal"				},
		{ TextureType::NORMAL,		"texture3_Normal",				"texture3_Normal"				},
		{ TextureType::NORMAL,		"Bricks_Normal",				"Bricks_Normal"					},
		{ TextureType::NORMAL,		"Shapes_Normal",				"Shapes_Normal"					},
		{ TextureType::NORMAL,		"Metal_Normal",					"Metal_Normal"					},
		{ TextureType::NORMAL,		"Cobble_Normal",				"Cobble_Normal"					},

		{ TextureType::SPECULAR,	"Default_Specular",				"Black"							},
		{ TextureType::SPECULAR,	"Gray_Specular",				"Gray"							},
		{ TextureType::SPECULAR,	"White_Specular",				"White"							},
		{ TextureType::SPECULAR,	"Fade_Specular",				"Fade"							},
		{ TextureType::SPECULAR,	"Test_Specular",				"SpecularTest"					},
		{ TextureType::SPECULAR,	"Bricks_Specular",				"Bricks_Specular"				},
		{ TextureType::SPECULAR,	"Metal_Specular",				"Metal_Specular"				},
		{ TextureType::SPECULAR,	"Cobble_Specular",				"Cobble_Specular"				},

		{ TextureType::REFLECTIVE,	"Default_Reflective",			"Black"							},
		{ TextureType::REFLECTIVE,	"Gray_Reflective",				"Gray"							},
		{ TextureType::REFLECTIVE,	"White_Reflective",				"White"							},
		{ TextureType::REFLECTIVE,	"Fade_Reflective",				"Fade"							},
		{ TextureType::REFLECTIVE,	"Metal_Reflective",				"Metal_Reflective"				},
		{ TextureType::REFLECTIVE,	"Cobble_Reflective",			"Cobble_Reflective"				},

		{ TextureType::HEIGHT,		"Default_Height",				"Black"							},
		{ TextureType::HEIGHT,		"Metal_Height",					"Metal_Height"					},
		{ TextureType::HEIGHT,		"Cobble_Height",				"Cobble_Height"					},
	};

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


	if (!LoadContent(time, meshNames, textureNames, textureMapNames, shaderNames))
	{
		ErrMsg("Failed to load game content!");
		return false;
	}

	return true;
}

bool Game::SetScene(Time& time, Scene* scene)
{
	if (scene == nullptr)
		return false;

	_scene = scene;

	if (!_scene->Initialize(_device, &_content, &_graphics))
	{
		ErrMsg("Failed to initialize scene!");
		return false;
	}

	return true;
}


bool Game::Update(Time& time, const Input& input) const
{
	time.TakeSnapshot("SceneUpdateTime");
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
	time.TakeSnapshot("SceneUpdateTime");

	return true;
}


bool Game::Render(Time& time, const Input& input)
{
	if (!_graphics.BeginSceneRender())
	{
		ErrMsg("Failed to begin rendering!");
		return false;
	}

	time.TakeSnapshot("SceneRenderTime");
	/// v==========================================v ///
	/// v        Render scene here...              v ///
	/// v==========================================v ///

	if (_scene != nullptr)
		if (!_scene->Render(time, input))
		{
			ErrMsg("Failed to render scene!");
			return false;
		}

	/// ^==========================================^ ///
	/// ^        Render scene here...              ^ ///
	/// ^==========================================^ ///
	time.TakeSnapshot("SceneRenderTime");

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

	//static float imGuiFontScale = 1.25f; // I have a 1440p monitor, shit's too small at default scale
	static float imGuiFontScale = 1.0f;
	ImGui::SetWindowFontScale(imGuiFontScale);

	/// v==========================================v ///
	/// v        Render UI here...                 v ///
	/// v==========================================v ///

	// For an in-depth manual of ImGui features and their usages see:
	// https://pthom.github.io/imgui_manual_online/manual/imgui_manual.html

	if (ImGui::CollapsingHeader("General"))
	{
		ImGui::SliderFloat("Font Scale", &imGuiFontScale, 0.25f, 8.0f);

		if (ImGui::Button("Reset Font Scale"))
			imGuiFontScale = 1.0f;

		ImGui::SetWindowFontScale(imGuiFontScale);
	}

	ImGui::Separator();

	if (ImGui::CollapsingHeader("Performance"))
	{
		if (ImGui::TreeNode("FPS"))
		{
			constexpr size_t FPS_BUF_SIZE = 128;
			static float fpsBuf[FPS_BUF_SIZE]{};
			static size_t fpsBufIndex = 0;

			float currFps = 1.0f / time.deltaTime;
			fpsBuf[fpsBufIndex] = currFps;
			(++fpsBufIndex) %= FPS_BUF_SIZE;

			float avgFps = 0.0f;
			float dropFps = FLT_MAX;
			for (size_t i = 0; i < FPS_BUF_SIZE; i++)
			{
				avgFps += fpsBuf[i];

				if (dropFps > fpsBuf[i])
					dropFps = fpsBuf[i];
			}
			avgFps /= FPS_BUF_SIZE;

			static float minFPS = FLT_MAX;
			if (minFPS > currFps)
				minFPS = currFps;

			char fps[8]{};
			snprintf(fps, sizeof(fps), "%.2f", currFps);
			ImGui::Text(std::format("FPS: {}", fps).c_str());

			snprintf(fps, sizeof(fps), "%.2f", avgFps);
			ImGui::Text(std::format("Avg: {}", fps).c_str());

			snprintf(fps, sizeof(fps), "%.2f", dropFps);
			ImGui::Text(std::format("Drop: {}", fps).c_str());

			snprintf(fps, sizeof(fps), "%.2f", minFPS);
			ImGui::Text(std::format("Min: {}", fps).c_str());

			if (ImGui::Button("Reset"))
			{
				minFPS = 1.0f / time.deltaTime;

				for (size_t i = 0; i < FPS_BUF_SIZE; i++)
					fpsBuf[i] = 0.0f;
			}

			ImGui::TreePop();
		}
		
		if (ImGui::TreeNode("Culling"))
		{
			char timeStr[32]{};

			snprintf(timeStr, sizeof(timeStr), "%.6f", time.deltaTime);
			ImGui::Text(std::format("{} Frame", timeStr).c_str());

			ImGui::Spacing();

			snprintf(timeStr, sizeof(timeStr), "%.6f", time.CompareSnapshots("SceneUpdateTime"));
			ImGui::Text(std::format("{} Scene Update", timeStr).c_str());

			ImGui::Spacing();

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

			snprintf(timeStr, sizeof(timeStr), "%.6f", time.CompareSnapshots("FrustumCullDirlights"));
			ImGui::Text(std::format("{} Culling Dirlights Total", timeStr).c_str());


			snprintf(timeStr, sizeof(timeStr), "%.6f", time.CompareSnapshots("FrustumCullPointlights"));
			ImGui::Text(std::format("{} Culling Pointlights Total", timeStr).c_str());

			i = 0;
			float pointlightTime = time.CompareSnapshots(std::format("FrustumCullPointlight{}", i));
			while (pointlightTime >= 0.0f)
			{
				snprintf(timeStr, sizeof(timeStr), "%.6f", pointlightTime);
				ImGui::Text(std::format("{} Culling Pointlight #{}", timeStr, i).c_str());

				pointlightTime = time.CompareSnapshots(std::format("FrustumCullPointlight{}", ++i));
			}

			ImGui::TreePop();
		}
	}

	ImGui::Separator();

	if (ImGui::CollapsingHeader("Graphics"))
	{
		if (!_graphics.RenderUI(time))
		{
			ErrMsg("Failed to render graphics UI!");
			return false;
		}
	}

	ImGui::Separator();

	if (ImGui::CollapsingHeader("Scene"))
	{
		if (_scene != nullptr)
			if (!_scene->RenderUI())
			{
				ErrMsg("Failed to render scene UI!");
				return false;
			}
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