#pragma once

#include "Transform.h"


class Entity
{
private:

public:
	Transform transform;

	Entity();
	~Entity();

	void Update();
	void Render();
};