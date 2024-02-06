#include "Scene.h"


Scene::Scene()
{
	// TODO
}

Scene::~Scene()
{
	// TODO
}


bool Scene::MakeQuad(const XMMATRIX &worldMatrix)
{
	_entities.push_back(Entity(worldMatrix));
	return true;
}