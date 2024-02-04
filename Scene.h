#pragma once

#include <vector>

#include "Entity.h"


class Scene
{
private:
	std::vector<Entity> _entities;

public:
	Scene();
	~Scene();
};