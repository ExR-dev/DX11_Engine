#include "SceneHolder.h"

#include "ErrMsg.h"


SceneHolder::~SceneHolder()
{
	for (const SceneEntity *ent : _entities)
		delete ent;
}

bool SceneHolder::Initialize(const DirectX::BoundingBox &sceneBounds)
{
	if (!_volumeTree.Initialize(sceneBounds))
	{
		ErrMsg("Failed to initialize volume tree!");
		return false;
	}

	return true;
}

bool SceneHolder::Update()
{
	for (const UINT i : _treeInsertionQueue)
	{
		Entity *entity = _entities[i]->item;

		DirectX::BoundingBox entityBounds;
		if (!entity->StoreBounds(entityBounds))
		{
			ErrMsg("Failed to insert queued entity into volume tree, unable to get entity bounds!");
			return false;
		}

		_volumeTree.Insert((IEntity *)entity, entityBounds);
	}
	_treeInsertionQueue.clear();

	return true;
}


// Entity is Not initialized automatically. Initialize manually through the returned pointer.
Entity *SceneHolder::AddEntity(const DirectX::BoundingBox &bounds)
{
	SceneEntity *newEntity = new SceneEntity(static_cast<UINT>(_entities.size()), bounds);
	_entities.push_back(newEntity);
	_treeInsertionQueue.push_back(newEntity->item->GetID());

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

	if (!_volumeTree.Remove((IEntity *)entity, entityBounds))
	{
		ErrMsg("Failed to remove entity from volume tree!");
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

	if (!_volumeTree.Remove((IEntity *)entity))
	{
		ErrMsg("Failed to remove entity from volume tree!");
		return false;
	}

	_volumeTree.Insert((IEntity *)entity, entityBounds);
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
	return static_cast<UINT>(_entities.size());
}

void SceneHolder::GetEntities(std::vector<Entity *> entities) const
{
	for (const SceneEntity *ent : _entities)
		entities.push_back(ent->item);
}


bool SceneHolder::FrustumCull(const DirectX::BoundingFrustum &frustum, std::vector<Entity *> &containingItems) const
{
	std::vector<IEntity *> containingInterfaces;
	containingInterfaces.reserve(_entities.capacity());

	if (!_volumeTree.FrustumCull(frustum, containingInterfaces))
	{
		ErrMsg("Failed to frustum cull volume tree!");
		return false;
	}

	for (IEntity *iEnt : containingInterfaces)
		containingItems.push_back((Entity *)iEnt);

	return true;
}

