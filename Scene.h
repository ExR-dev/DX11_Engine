#pragma once

#include <vector>
#include <d3d11.h>
#include <DirectXMath.h>

#include "Entity.h"
#include "DebugObject.h"

using namespace DirectX;


class Scene
{
private:
	bool _initialized;

	std::vector<Entity> _entities;
	std::vector<DebugObject> _debugObjects;

public:
	Scene();
	~Scene();

	bool Initialize(ID3D11Device *device);
	bool Uninitialize();

	bool Update(ID3D11DeviceContext *context, const Time &time);
	bool Render(ID3D11DeviceContext *context);
};