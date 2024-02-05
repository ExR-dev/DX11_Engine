#pragma once

#include "Transform.h"


class Entity
{
private:
	Transform transform;

public:
	Entity();
	~Entity();

	void Update();
	void Render();
};