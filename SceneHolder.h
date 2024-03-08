#pragma once

#include "Entity.h"
#include "Object.h"
//#include "Octree.h"
#include "Quadtree.h"


class SceneHolder
{
private:
	struct SceneEntity
	{
		union
		{
			Object *item = nullptr;
			//Emitter *item = nullptr;
			//Light *item = nullptr;
		};

		explicit SceneEntity(const UINT id, const DirectX::BoundingBox &bounds, EntityType type)
		{
			switch (type)
			{
				case EntityType::OBJECT:
					item = new Object(id, bounds);
					break;

				case EntityType::EMITTER:
					break;

				case EntityType::LIGHT:
					break;
			}
		}

		~SceneEntity()
		{ delete item; }

		SceneEntity(const SceneEntity &other) = delete;
		SceneEntity &operator=(const SceneEntity &other) = delete;
		SceneEntity(SceneEntity &&other) = delete;
		SceneEntity &operator=(SceneEntity &&other) = delete;
	};

	std::vector<SceneEntity *> _entities;

	//Octree _volumeTree;
	Quadtree _volumeTree;

	std::vector<UINT> _treeInsertionQueue;


public:
	SceneHolder() = default;
	~SceneHolder();
	SceneHolder(const SceneHolder &other) = delete;
	SceneHolder &operator=(const SceneHolder &other) = delete;
	SceneHolder(SceneHolder &&other) = delete;
	SceneHolder &operator=(SceneHolder &&other) = delete;

	[[nodiscard]] bool Initialize(const DirectX::BoundingBox &sceneBounds);
	[[nodiscard]] bool Update();

	[[nodiscard]] Entity *AddEntity(const BoundingBox &bounds, EntityType type);
	[[nodiscard]] bool RemoveEntity(Entity *entity);
	[[nodiscard]] bool RemoveEntity(UINT id);

	[[nodiscard]] bool UpdateEntityPosition(Entity *entity);

	[[nodiscard]] Entity *GetEntity(UINT id) const;
	[[nodiscard]] UINT GetEntityCount() const;
	void GetEntities(std::vector<Entity *> entities) const;

	[[nodiscard]] bool FrustumCull(const DirectX::BoundingFrustum &frustum, std::vector<Entity *> &containingItems) const;
};
