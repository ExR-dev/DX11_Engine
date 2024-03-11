#pragma once

#include <memory>
#include <DirectXCollision.h>
#include <vector>

#include "Entity.h"


class Quadtree
{
private:
	static constexpr UINT MAX_ITEMS_IN_NODE = 24;
	static constexpr UINT MAX_DEPTH = 5;


	struct Node
	{
		std::vector<Entity *> data;
		DirectX::BoundingBox bounds;
		std::unique_ptr<Node> children[4];
		bool isLeaf = true;


		void Split(const UINT depth)
		{
			const DirectX::XMFLOAT3
				center = bounds.Center,
				extents = bounds.Extents,
				min = { center.x - extents.x, 0.0f, center.z - extents.z },
				max = { center.x + extents.x, 0.0f, center.z + extents.z };

			children[0] = std::make_unique<Node>();
			children[1] = std::make_unique<Node>();
			children[2] = std::make_unique<Node>();
			children[3] = std::make_unique<Node>();

			DirectX::BoundingBox::CreateFromPoints(children[0]->bounds, { min.x, -extents.y, min.z, 0 }, { center.x, extents.y, center.z, 0 });
			DirectX::BoundingBox::CreateFromPoints(children[1]->bounds, { center.x, -extents.y, min.z, 0 }, { max.x, extents.y, center.z, 0 });
			DirectX::BoundingBox::CreateFromPoints(children[2]->bounds, { min.x, -extents.y, center.z, 0 }, { center.x, extents.y, max.z, 0 });
			DirectX::BoundingBox::CreateFromPoints(children[3]->bounds, { center.x, -extents.y, center.z, 0 }, { max.x, extents.y, max.z, 0 });

			for (int i = 0; i < data.size(); i++)
				if (data[i] != nullptr)
				{
					DirectX::BoundingBox itemBounds;
					data[i]->StoreBounds(itemBounds);

					for (int j = 0; j < 4; j++)
						children[j]->Insert(data[i], itemBounds, depth + 1);
				}

			data.clear();
			isLeaf = false;
		}


		bool Insert(Entity *item, const DirectX::BoundingBox &itemBounds, const UINT depth = 0)
		{
			if (!bounds.Intersects(itemBounds))
				return false;

			if (isLeaf)
			{
				if (depth >= MAX_DEPTH || data.size() < MAX_ITEMS_IN_NODE)
				{
					data.push_back(item);
					return true;
				}

				Split(depth);
			}

			for (int i = 0; i < 4; i++)
			{
				if (children[i] != nullptr)
					children[i]->Insert(item, itemBounds, depth + 1);
			}

			return true;
		}

		void Remove(Entity *item, const DirectX::BoundingBox &itemBounds, const UINT depth = 0, const bool skipIntersection = false)
		{
			if (!skipIntersection)
				if (!bounds.Intersects(itemBounds))
					return;

			if (isLeaf)
			{
				std::erase_if(data, [item](const Entity *otherItem) { return item == otherItem; });
				return;
			}

			for (int i = 0; i < 4; i++)
			{
				if (children[i] != nullptr)
					children[i]->Remove(item, itemBounds, depth + 1, skipIntersection);
			}

			std::vector<Entity *> containingItems;
			containingItems.reserve(MAX_ITEMS_IN_NODE);
			for (int i = 0; i < 4; i++)
				if (children[i] != nullptr)
				{
					if (!children[i]->isLeaf)
						return;

					if (!children[i]->data.empty())
					{
						for (Entity *childItem : children[i]->data)
						{
							if (childItem == nullptr)
								continue;

							if (std::ranges::find(containingItems, childItem) == containingItems.end())
							{
								if (containingItems.size() >= MAX_ITEMS_IN_NODE)
									return;

								containingItems.push_back(childItem);
							}
						}
					}
				}

			for (int i = 0; i < 4; i++)
			{
				children[i].release();
				children[i] = nullptr;
			}

			isLeaf = true;
			data.clear();
			for (Entity *newItem : containingItems)
				data.push_back(newItem);
		}


		void FrustumCull(const DirectX::BoundingFrustum &frustum, std::vector<Entity *> &containingItems, const UINT depth = 0) const
		{
			if (!frustum.Intersects(bounds))
				return;

			if (isLeaf)
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

			for (int i = 0; i < 4; i++)
			{
				if (children[i] == nullptr)
					continue;

				children[i]->FrustumCull(frustum, containingItems, depth + 1);
			}
		}
	};

	std::unique_ptr<Node> _root;


public:
	Quadtree() = default;
	~Quadtree() = default;
	Quadtree(const Quadtree &other) = delete;
	Quadtree &operator=(const Quadtree &other) = delete;
	Quadtree(Quadtree &&other) = delete;
	Quadtree &operator=(Quadtree &&other) = delete;

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

		_root->Remove(data, _root->bounds, 0, true);
		return true;
	}

	[[nodiscard]] bool FrustumCull(const DirectX::BoundingFrustum &frustum, std::vector<Entity *> &containingItems) const
	{
		if (_root == nullptr)
			return false;

		_root->FrustumCull(frustum, containingItems);
		return true;
	}

	[[nodiscard]] DirectX::BoundingBox *GetBounds() const
	{
		if (_root == nullptr)
			return nullptr;

		return &_root->bounds;
	}
};