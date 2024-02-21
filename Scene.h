#pragma once

#include <vector>
#include <d3d11.h>

#include "CameraD3D11.h"
#include "Entity.h"
#include "Input.h"


class Scene
{
private:
	bool _initialized;

	CameraD3D11 *_camera;
	std::vector<Entity *> _entities;

public:
	Scene();
	~Scene();
	Scene(const Scene &other) = delete;
	Scene &operator=(const Scene &other) = delete;
	Scene(Scene &&other) = delete;
	Scene &operator=(Scene &&other) = delete;

	[[nodiscard]] bool Initialize(ID3D11Device *device);

	[[nodiscard]] bool Update(ID3D11DeviceContext *context, const Time &time, const Input &input);
	[[nodiscard]] bool Render(Graphics *graphics);
};