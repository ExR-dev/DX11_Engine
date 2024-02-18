#pragma once

#include "Transform.h"
#include "Time.h"
#include "Content.h"


class Entity
{
private:
	Transform _transform;
	MeshD3D11 *_mesh = nullptr;

public:
	Entity();
	explicit Entity(const XMMATRIX &worldMatrix);
	~Entity();
	Entity(const Entity &other) = delete;
	Entity &operator=(const Entity &other) = delete;
	Entity(Entity &&other) = default;
	Entity &operator=(Entity &&other) = delete;

	bool Update(const Time &time);
	bool Render(ID3D11DeviceContext *context);
};