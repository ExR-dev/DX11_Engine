#pragma once

#include <memory>
#include <utility>
#include <vector>
#include <DirectXCollision.h>

#include "Entity.h"
#include "Raycast.h"


class Notree
{
private:
	struct Node
	{
		std::vector<Entity *> data;
		DirectX::BoundingBox bounds;


		bool Insert(Entity *item, const DirectX::BoundingBox &itemBounds)
		{
			if (!bounds.Intersects(itemBounds))
				return false;

			data.push_back(item);
			return true;
		}

		void Remove(Entity *item, const DirectX::BoundingBox &itemBounds, const bool skipIntersection = false)
		{
			if (!skipIntersection)
				if (!bounds.Intersects(itemBounds))
					return;

			std::erase_if(data, [item](const Entity *otherItem) { return item == otherItem; });
			return;
		}


		void AddToVector(std::vector<Entity *> &containingItems) const
		{
			for (Entity *item : data)
			{
				if (item == nullptr)
					continue;

				if (std::ranges::find(containingItems, item) == containingItems.end())
					containingItems.push_back(item);
			}

			return;
		}

		void FrustumCull(const DirectX::BoundingFrustum &frustum, std::vector<Entity *> &containingItems) const
		{
			switch (frustum.Contains(bounds))
			{
				case DirectX::DISJOINT:
					return;

				case DirectX::CONTAINS:
					AddToVector(containingItems);
					break;

				case DirectX::INTERSECTS:
					for (Entity *item : data)
					{
						if (item == nullptr)
							continue;

						if (std::ranges::find(containingItems, item) == containingItems.end())
							containingItems.push_back(item);
					}
					return;
			}
		}

		void BoxCull(const DirectX::BoundingOrientedBox &box, std::vector<Entity *> &containingItems) const
		{
			switch (box.Contains(bounds))
			{
				case DirectX::DISJOINT:
					return;

				case DirectX::CONTAINS:
					AddToVector(containingItems);
					break;

				case DirectX::INTERSECTS:
					for (Entity *item : data)
					{
						if (item == nullptr)
							continue;

						if (std::ranges::find(containingItems, item) == containingItems.end())
							containingItems.push_back(item);
					}
					return;
			}
		}


		bool RaycastNode(const DirectX::XMFLOAT3 &orig, const DirectX::XMFLOAT3 &dir, float &length, Entity *&entity) const
		{
			// Check all items in leaf for intersection & return result.
			for (Entity *item : data)
			{
				if (item == nullptr)
					continue;

				DirectX::BoundingBox itemBounds;
				item->StoreBounds(itemBounds);

				float newLength = 0.0f;
				if (Raycast(orig, dir, itemBounds, newLength))
				{
					if (newLength < 0.0f)
						continue;

					if (newLength >= length)
						continue;

					length = newLength;
					entity = item;
				}
			}

			return (entity != nullptr);
		}


		void DebugGetStructure(std::vector<DirectX::BoundingBox> &boxCollection) const
		{
			boxCollection.push_back(bounds);
			return;
		}
	};

	std::unique_ptr<Node> _root;


public:
	Notree() = default;
	~Notree() = default;
	Notree(const Notree &other) = delete;
	Notree &operator=(const Notree &other) = delete;
	Notree(Notree &&other) = delete;
	Notree &operator=(Notree &&other) = delete;

	[[nodiscard]] bool Initialize(const DirectX::BoundingBox &sceneBounds)
	{
		_root = std::make_unique<Node>();
		_root->bounds = sceneBounds;

		return true;
	}

	void Insert(Entity *data, const DirectX::BoundingBox &bounds) const
	{
		if (_root != nullptr)
			_root->Insert(data, bounds);
	}


	[[nodiscard]] bool Remove(Entity *data, const DirectX::BoundingBox &bounds) const
	{
		if (_root == nullptr)
			return false;

		_root->Remove(data, bounds);
		return true;
	}

	[[nodiscard]] bool Remove(Entity *data) const
	{
		if (_root == nullptr)
			return false;

		_root->Remove(data, _root->bounds, true);
		return true;
	}


	[[nodiscard]] bool FrustumCull(const DirectX::BoundingFrustum &frustum, std::vector<Entity *> &containingItems) const
	{
		if (_root == nullptr)
			return false;

		_root->FrustumCull(frustum, containingItems);
		return true;
	}

	[[nodiscard]] bool BoxCull(const DirectX::BoundingOrientedBox &box, std::vector<Entity *> &containingItems) const
	{
		if (_root == nullptr)
			return false;

		_root->BoxCull(box, containingItems);
		return true;
	}


	bool RaycastTree(const DirectX::XMFLOAT3A &orig, const DirectX::XMFLOAT3A &dir, float &length, Entity *&entity) const
	{
		if (_root == nullptr)
			return false;

		float _ = FLT_MAX; // In case Intersects() uses the initial dist value as a maximum. Docs don't specify.
		if (!Raycast(orig, dir, _root->bounds, _))
			return false;

		length = FLT_MAX;
		entity = nullptr;

		return _root->RaycastNode(orig, dir, length, entity);
	}


	[[nodiscard]] DirectX::BoundingBox *GetBounds() const
	{
		if (_root == nullptr)
			return nullptr;

		return &_root->bounds;
	}


	void DebugGetStructure(std::vector<DirectX::BoundingBox> &boxCollection) const
	{
		if (_root == nullptr)
			return;

		_root->DebugGetStructure(boxCollection);
	}
};