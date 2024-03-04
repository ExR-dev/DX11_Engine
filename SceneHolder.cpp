#include "SceneHolder.h"

#include "ErrMsg.h"


SceneHolder::~SceneHolder()
{
	for (const SceneEntity *ent : _entities)
		delete ent;
}

bool SceneHolder::Initialize(const DirectX::BoundingBox &sceneBounds)
{
	if (!_octree.Initialize(sceneBounds))
	{
		ErrMsg("Failed to initialize octree!");
		return false;
	}

	return true;
}


// Entity is Not initialized automatically. Initialize manually through the returned pointer.
Entity *SceneHolder::AddEntity(const DirectX::BoundingBox &bounds)
{
	SceneEntity *newEntity = new SceneEntity(_entities.size(), bounds);
	_entities.push_back(newEntity);
	_octree.Insert(_entities.back()->item, bounds);

	return _entities.back()->item;
}

bool SceneHolder::RemoveEntity(Entity *entity)
{
	DirectX::BoundingBox entityBounds;
	if (!entity->StoreBounds(entityBounds))
	{
		ErrMsg("Failed to remove entity, unable to get entity bounds!");
		return false;
	}

	if (!_octree.Remove(entity, entityBounds))
	{
		ErrMsg("Failed to remove entity from octree!");
		return false;
	}

	std::erase_if(_entities, [entity](const SceneEntity *item) { return item->item == entity; });
	return true;
}

bool SceneHolder::RemoveEntity(const UINT id)
{
	if (id >= _entities.size())
	{
		ErrMsg("Failed to remove entity, ID out of range!");
		return false;
	}

	return RemoveEntity(_entities[id]->item);
}


bool SceneHolder::UpdateEntityPosition(Entity *entity)
{
	DirectX::BoundingBox entityBounds;
	if (!entity->StoreBounds(entityBounds))
	{
		ErrMsg("Failed to update entity position, unable to get entity bounds!");
		return false;
	}

	if (!_octree.Remove(entity))
	{
		ErrMsg("Failed to remove entity from octree!");
		return false;
	}

	_octree.Insert(entity, entityBounds);
	return true;
}


Entity *SceneHolder::GetEntity(const UINT id) const
{
	if (id >= _entities.size())
	{
		ErrMsg("Failed to get entity, ID out of range!");
		return nullptr;
	}

	return _entities[id]->item;
}

UINT SceneHolder::GetEntityCount() const
{
	return _entities.size();
}

void SceneHolder::GetEntities(std::vector<Entity *> entities) const
{
	for (const SceneEntity *ent : _entities)
		entities.push_back(ent->item);
}


bool SceneHolder::FrustumCull(const DirectX::BoundingFrustum &frustum, std::set<Entity *> &containingItems)
{
	return _octree.FrustumCull(frustum, containingItems);
}

