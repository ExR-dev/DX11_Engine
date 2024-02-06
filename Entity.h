#pragma once

#include "Transform.h"


class Entity
{
private:
	Transform _transform;

public:
	Entity();
	Entity(const XMMATRIX &worldMatrix);
	~Entity();

	void Update();
	void Render();
};