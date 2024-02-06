#pragma once

#include <vector>
#include <DirectXMath.h>

#include "Entity.h"

using namespace DirectX;


class Scene
{
private:
	std::vector<Entity> _entities;

public:
	Scene();
	~Scene();

	bool MakeQuad(const XMMATRIX &worldMatrix);
};