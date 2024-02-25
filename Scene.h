#pragma once

#include <vector>
#include <d3d11.h>

#include "CameraD3D11.h"
#include "Entity.h"
#include "Input.h"
#include "SpotLightCollectionD3D11.h"
#include "Cubemap.h"


class Scene
{
private:
	bool _initialized;
	ID3D11Device *_device = nullptr;

	CameraD3D11 *_camera = nullptr;
	std::vector<Entity *> _entities;
	SpotLightCollectionD3D11 *_spotLights;

	Cubemap _cubemap;

public:
	Scene();
	~Scene();
	Scene(const Scene &other) = delete;
	Scene &operator=(const Scene &other) = delete;
	Scene(Scene &&other) = delete;
	Scene &operator=(Scene &&other) = delete;

	[[nodiscard]] bool Initialize(ID3D11Device *device);

	[[nodiscard]] bool Update(ID3D11DeviceContext *context, const Time &time, const Input &input);
	[[nodiscard]] bool Render(Graphics *graphics, const Time &time, const Input &input);
};
