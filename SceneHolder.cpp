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
		
		DirectX::BoundingBox entityBounds;
		entity->StoreBounds(entityBounds);

		_volumeTree.Insert(entity, entityBounds);
	}
	_treeInsertionQueue.clear();

	return true;
}


// Entity is Not initialized automatically. Initialize manually through the returned pointer.
Entity *SceneHolder::AddEntity(const DirectX::BoundingBox &bounds, const EntityType type)
{
	SceneEntity *newEntity = new SceneEntity(_entityCounter, bounds, type);
	_entities.push_back(newEntity);
	_entityCounter++;

	switch (type)
	{
		case EntityType::OBJECT:
			_treeInsertionQueue.push_back((reinterpret_cast<Entity *>(newEntity->_item.object))->GetID());
			return reinterpret_cast<Entity *>(_entities.back()->_item.object);

		case EntityType::EMITTER:
			_treeInsertionQueue.push_back((reinterpret_cast<Entity *>(newEntity->_item.emitter))->GetID());
			return reinterpret_cast<Entity *>(_entities.back()->_item.emitter);
	}

	return nullptr;
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

	DirectX::BoundingBox entityBounds;
	entity->StoreBounds(entityBounds);

	if (!_volumeTree.Remove(entity, entityBounds))
	{
		ErrMsg("Failed to remove entity from volume tree!");
		return false;
	}

	switch (entity->GetType())
	{
		case EntityType::OBJECT:
			std::erase_if(_entities, [entity](const SceneEntity *item) { return reinterpret_cast<Entity *>(item->_item.object) == entity; });
			break;

		case EntityType::EMITTER:
			std::erase_if(_entities, [entity](const SceneEntity *item) { return reinterpret_cast<Entity *>(item->_item.emitter) == entity; });
			break;
	}

	return true;
}

bool SceneHolder::RemoveEntity(const UINT id)
{
	if (id >= _entities.size())
	{
		ErrMsg("Failed to remove entity, ID out of range!");
		return false;
	}

	switch (_entities[id]->_type)
	{
		case EntityType::OBJECT:
			return RemoveEntity(reinterpret_cast<Entity *>(_entities[id]->_item.object));

		case EntityType::EMITTER:
			return RemoveEntity(reinterpret_cast<Entity *>(_entities[id]->_item.emitter));
	}

	return false;
}


bool SceneHolder::UpdateEntityPosition(Entity *entity)
{
	DirectX::BoundingBox entityBounds;
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
	if (i >= _entities.size())
	{
		ErrMsg("Failed to get entity, i out of range!");
		return nullptr;
	}

	switch (_entities[i]->_type)
	{
		case EntityType::OBJECT:
			return reinterpret_cast<Entity *>(_entities[i]->_item.object);

		case EntityType::EMITTER:
			return reinterpret_cast<Entity *>(_entities[i]->_item.emitter);
	}

	return nullptr;
}

Entity *SceneHolder::GetEntityByID(const UINT id) const
{
	const UINT entityCount = GetEntityCount();
	for (UINT i = 0; i < entityCount; i++)
	{
		switch (_entities[i]->_type)
		{
			case EntityType::OBJECT:
				if (reinterpret_cast<Entity *>(_entities[i]->_item.object)->GetID() != id)
					break;
				return reinterpret_cast<Entity *>(_entities[i]->_item.object);

			case EntityType::EMITTER:
				if (reinterpret_cast<Entity *>(_entities[i]->_item.emitter)->GetID() != id)
					break;
				return reinterpret_cast<Entity *>(_entities[i]->_item.emitter);
		}
	}

	return nullptr;
}

Entity *SceneHolder::GetEntityByName(const std::string &name) const
{
	const UINT entityCount = GetEntityCount();
	for (UINT i = 0; i < entityCount; i++)
	{
		switch (_entities[i]->_type)
		{
			case EntityType::OBJECT:
				if (reinterpret_cast<Entity *>(_entities[i]->_item.object)->GetName() == name)
					return reinterpret_cast<Entity *>(_entities[i]->_item.object);
				break;

			case EntityType::EMITTER:
				if (reinterpret_cast<Entity *>(_entities[i]->_item.emitter)->GetName() == name)
					return reinterpret_cast<Entity *>(_entities[i]->_item.emitter);
				break;
		}
	}

	return nullptr;
}

UINT SceneHolder::GetEntityIndex(const Entity *entity) const
{
	const UINT entityCount = GetEntityCount();
	for (UINT i = 0; i < entityCount; i++)
	{
		switch (_entities[i]->_type)
		{
			case EntityType::OBJECT:
				if (entity == reinterpret_cast<Entity *>(_entities[i]->_item.object))
					return i;
				break;

			case EntityType::EMITTER:
				if (entity == reinterpret_cast<Entity *>(_entities[i]->_item.emitter))
					return i;
				break;
		}
	}

	return 0xffffffff;
}

UINT SceneHolder::GetEntityCount() const
{
	return static_cast<UINT>(_entities.size());
}

void SceneHolder::GetEntities(std::vector<Entity *> entities) const
{
	for (const SceneEntity *ent : _entities)
		switch (ent->_type)
		{
			case EntityType::OBJECT:
				entities.push_back(reinterpret_cast<Entity *>(ent->_item.object));
				break;

			case EntityType::EMITTER:
				entities.push_back(reinterpret_cast<Entity *>(ent->_item.emitter));
				break;
		}
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

#ifdef QUADTREE_CULLING
#elif defined OCTREE_CULLING
#endif
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
#ifdef QUADTREE_CULLING
#elif defined OCTREE_CULLING
#endif
	return _volumeTree.RaycastTree(origin, direction, result.distance, result.entity);
}


void SceneHolder::DebugGetTreeStructure(std::vector<DirectX::BoundingBox> &boxCollection) const
{
#ifdef QUADTREE_CULLING
#elif defined OCTREE_CULLING
#endif
	_volumeTree.DebugGetStructure(boxCollection);
}
