#pragma once

#include <d3d11.h>

#include "Scene.h"
#include "Graphics.h"
#include "Content.h"
#include "Time.h"
#include "Input.h"


// Game handles loading content like textures and meshes, as well as managing the update and render steps of the main game loop.
class Game
{
private:
	ID3D11Device		*_device;
	ID3D11DeviceContext *_immediateContext;

	Graphics	_graphics;
	Content		_content;
	Scene		*_scene;

	struct TextureData		{					std::string name;	std::string file; };
	struct TextureMapData	{ TextureType type;	std::string name;	std::string file; };
	struct ShaderData		{ ShaderType type;	std::string name;	std::string file; };

	[[nodiscard]] bool LoadContent(Time &time,
		const std::vector<std::string> &meshNames,
		const std::vector<TextureData> &textureNames, 
		const std::vector<TextureMapData> &textureMapNames, 
		const std::vector<ShaderData> &shaderNames
	);

public:
	Game();
	~Game();

	[[nodiscard]] bool Setup(Time &time, UINT width, UINT height, HWND window);
	[[nodiscard]] bool SetScene(Time &time, Scene *scene);

	[[nodiscard]] bool Update(Time &time, const Input &input) const;
	[[nodiscard]] bool Render(Time &time, const Input &input);
};
