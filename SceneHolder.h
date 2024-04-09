#pragma once

#include "Entity.h"
#include "Object.h"
#include "Emitter.h"
//#include "Quadtree.h"
#include "Octree.h"


struct RaycastOut
{
	Entity *entity = nullptr;
	float distance = FLT_MAX;
};


class SceneHolder
{
private:
	struct SceneEntity
	{
		EntityType _type = EntityType::OBJECT;

		union
		{
			Object *object = nullptr;
			Emitter *emitter;
		} _item;

		explicit SceneEntity(const UINT id, const DirectX::BoundingBox &bounds, EntityType type)
		{
			_type = type;

			switch (type)
			{
				case EntityType::OBJECT:
					_item.object = new Object(id, bounds);
					break;

				case EntityType::EMITTER:
					_item.emitter = new Emitter(id, bounds);
					break;
			}
		}

		~SceneEntity()
		{
			switch (_type)
			{
				case EntityType::OBJECT:
					delete _item.object;
					break;

				case EntityType::EMITTER:
					delete _item.emitter;
					break;
			}
		}

		SceneEntity(const SceneEntity &other) = delete;
		SceneEntity &operator=(const SceneEntity &other) = delete;
		SceneEntity(SceneEntity &&other) = delete;
		SceneEntity &operator=(SceneEntity &&other) = delete;
	};

	UINT _entityCounter = 0;

	DirectX::BoundingBox _bounds;
	std::vector<SceneEntity *> _entities; 

	Octree _volumeTree;
	//Quadtree _volumeTree;

	std::vector<UINT> _treeInsertionQueue;


public:
	enum BoundsType {
		Frustum		= 0,
		OrientedBox = 1
	};

	SceneHolder() = default;
	~SceneHolder();
	SceneHolder(const SceneHolder &other) = delete;
	SceneHolder &operator=(const SceneHolder &other) = delete;
	SceneHolder(SceneHolder &&other) = delete;
	SceneHolder &operator=(SceneHolder &&other) = delete;

	[[nodiscard]] bool Initialize(const DirectX::BoundingBox &sceneBounds);
	[[nodiscard]] bool Update();

	[[nodiscard]] Entity *AddEntity(const DirectX::BoundingBox &bounds, EntityType type);
	[[nodiscard]] bool RemoveEntity(Entity *entity);
	[[nodiscard]] bool RemoveEntity(UINT id);

	[[nodiscard]] bool UpdateEntityPosition(Entity *entity);

	[[nodiscard]] const DirectX::BoundingBox &GetBounds() const;
	[[nodiscard]] Entity *GetEntity(UINT i) const;
	[[nodiscard]] Entity *GetEntityByID(UINT id) const;
	[[nodiscard]] UINT GetEntityID(const Entity *entity) const;
	[[nodiscard]] UINT GetEntityIndex(const Entity *entity) const;
	[[nodiscard]] UINT GetEntityCount() const;
	void GetEntities(std::vector<Entity *> entities) const;

	[[nodiscard]] bool FrustumCull(const DirectX::BoundingFrustum &frustum, std::vector<Entity *> &containingItems) const;
	[[nodiscard]] bool BoxCull(const DirectX::BoundingOrientedBox &box, std::vector<Entity *> &containingItems) const;

	bool Raycast(const DirectX::XMFLOAT3A &origin, const DirectX::XMFLOAT3A &direction, RaycastOut &result) const;

	void DebugGetTreeStructure(std::vector<DirectX::BoundingBox> &boxCollection) const;
};
