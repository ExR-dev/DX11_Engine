#include "SceneHolder.h"
#include "ErrMsg.h"

SceneHolder::~SceneHolder()
{
	for (const SceneEntity *ent : _entities)
		delete ent;
}
bool SceneHolder::Initialize(const DirectX::BoundingBox &sceneBounds)
{
	_bounds = sceneBounds;
	if (!_volumeTree.Initialize(sceneBounds))
	{
		ErrMsg("Failed to initialize volume tree!");
		return false;
	}

	return true;
}

bool SceneHolder::Update()
{
	for (const UINT id : _treeInsertionQueue)
	{
		Entity *entity = GetEntityByID(id);

		if (entity == nullptr)
		{
			ErrMsg(std::format("Failed to update scene holder, entity ID #{} not found!", id));
			return false;
		}
		
		DirectX::BoundingOrientedBox entityBounds;
		entity->StoreBounds(entityBounds);

		_volumeTree.Insert(entity, entityBounds);
	}
	_treeInsertionQueue.clear();

	return true;
}

Entity *SceneHolder::AddEntity(const DirectX::BoundingOrientedBox &bounds, const EntityType type)
{
	SceneEntity *newEntity = new SceneEntity(_entityCounter, bounds, type);
	_entities.push_back(newEntity);
	_entityCounter++;

	_treeInsertionQueue.push_back(newEntity->GetEntity()->GetID());
	return _entities.back()->GetEntity();
}
bool SceneHolder::RemoveEntity(Entity *entity)
{
	for (auto& child : *entity->GetChildren())
	{
		if (!RemoveEntity(child))
		{
			ErrMsg("Failed to remove child entity!");
			delete child;
			return false;
		}
		delete child;
	}

	DirectX::BoundingOrientedBox entityBounds;
	entity->StoreBounds(entityBounds);

	if (!_volumeTree.Remove(entity, entityBounds))
	{
		ErrMsg("Failed to remove entity from volume tree!");
		return false;
	}

	std::erase_if(_entities, [entity](const SceneEntity *item) { return item->GetEntity()->GetID() == entity->GetID(); });
	return true;
}
bool SceneHolder::RemoveEntity(const UINT index)
{
	if (index >= _entities.size())
	{
		ErrMsg("Failed to remove entity, ID out of range!");
		return false;
	}

	return RemoveEntity(_entities.at(index)->GetEntity());
}

bool SceneHolder::UpdateEntityPosition(Entity *entity)
{
	for (auto &child : *entity->GetChildren())
	{
		if (!UpdateEntityPosition(child))
		{
			ErrMsg("Failed to update child entity position!");
			return false;
		}
	}

	DirectX::BoundingOrientedBox entityBounds;
	entity->StoreBounds(entityBounds);

	if (!_volumeTree.Remove(entity))
	{
		ErrMsg("Failed to remove entity from volume tree!");
		return false;
	}

	_volumeTree.Insert(entity, entityBounds);
	return true;
}

const DirectX::BoundingBox& SceneHolder::GetBounds() const
{
	return _bounds;
}

Entity *SceneHolder::GetEntity(const UINT i) const
{
	if (i < 0)
	{
		ErrMsg("Failed to get entity, i out of range!");
		return nullptr;
	}

	if (i >= _entities.size())
	{
		ErrMsg("Failed to get entity, i out of range!");
		return nullptr;
	}

	return _entities[i]->GetEntity();
}
Entity *SceneHolder::GetEntityByID(const UINT id) const
{
	const UINT entityCount = GetEntityCount();
	for (UINT i = 0; i < entityCount; i++)
	{
		if (_entities[i]->GetEntity()->GetID() == id)
			return _entities[i]->GetEntity();
	}

	return nullptr;
}
Entity *SceneHolder::GetEntityByName(const std::string &name) const
{
	const UINT entityCount = GetEntityCount();
	for (UINT i = 0; i < entityCount; i++)
	{
		if (_entities[i]->GetEntity()->GetName() == name)
			return _entities[i]->GetEntity();
	}

	return nullptr;
}
void SceneHolder::GetEntities(std::vector<Entity *> &entities) const
{
	for (const SceneEntity *ent : _entities)
		entities.push_back(ent->GetEntity());
}

UINT SceneHolder::GetEntityIndex(const Entity *entity) const
{
	const UINT entityCount = GetEntityCount();
	for (UINT i = 0; i < entityCount; i++)
	{
		if (entity == _entities[i]->GetEntity())
			return i;
	}

	return -1;
}
UINT SceneHolder::GetEntityCount() const
{
	return static_cast<UINT>(_entities.size());
}

bool SceneHolder::FrustumCull(const DirectX::BoundingFrustum &frustum, std::vector<Entity *> &containingItems) const
{
	std::vector<Entity *> containingInterfaces;
	containingInterfaces.reserve(_entities.capacity());

	if (!_volumeTree.FrustumCull(frustum, containingInterfaces))
	{
		ErrMsg("Failed to frustum cull volume tree!");
		return false;
	}

	for (Entity *iEnt : containingInterfaces)
		containingItems.push_back(iEnt);

	return true;
}
bool SceneHolder::BoxCull(const DirectX::BoundingOrientedBox &box, std::vector<Entity *> &containingItems) const
{
	std::vector<Entity *> containingInterfaces;
	containingInterfaces.reserve(_entities.capacity());

	if (!_volumeTree.BoxCull(box, containingInterfaces))
	{
		ErrMsg("Failed to box cull volume tree!");
		return false;
	}

	for (Entity *iEnt : containingInterfaces)
		containingItems.push_back(iEnt);

	return true;
}
bool SceneHolder::Raycast(const DirectX::XMFLOAT3A &origin, const DirectX::XMFLOAT3A &direction, RaycastOut &result) const
{
	return _volumeTree.RaycastTree(origin, direction, result.distance, result.entity);
}

void SceneHolder::DebugGetTreeStructure(std::vector<DirectX::BoundingBox> &boxCollection) const
{
	_volumeTree.DebugGetStructure(boxCollection);
}
