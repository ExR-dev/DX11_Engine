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

bool Entity::Update(const Time &time)
{
	// TODO
	return false;
}

bool Entity::Render(ID3D11DeviceContext *context)
{
	// TODO
	return false;
}