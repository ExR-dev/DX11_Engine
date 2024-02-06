#include "Entity.h"


Entity::Entity()
{
	// TODO
}

Entity::Entity(const XMMATRIX &worldMatrix)
{
	_transform = Transform(worldMatrix);
}

Entity::~Entity()
{
	// TODO
}

void Entity::Update()
{
	// TODO
}

void Entity::Render()
{
	// TODO
}