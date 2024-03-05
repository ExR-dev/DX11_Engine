#pragma once

#include <memory>
#include <DirectXCollision.h>
#include <vector>
#include <omp.h>

#include "Entity.h"


class Octree
{
private:
	static constexpr UINT _MAX_ITEMS_IN_NODE = 8;
	static constexpr UINT _MAX_DEPTH = 3;
	static constexpr UINT _THREAD_DEPTH = 1;

	struct Node
	{
		std::vector<IEntity *> data;
		DirectX::BoundingBox bounds;
		std::unique_ptr<Node> children[8];
		bool isLeaf = true;


		void Split(const UINT depth)
		{
			const DirectX::XMFLOAT3
				center = bounds.Center,
				extents = bounds.Extents,
				min = { center.x - extents.x, center.y - extents.y, center.z - extents.z },
				max = { center.x + extents.x, center.y + extents.y, center.z + extents.z };

			children[0] = std::make_unique<Node>();
			children[1] = std::make_unique<Node>();
			children[2] = std::make_unique<Node>();
			children[3] = std::make_unique<Node>();
			children[4] = std::make_unique<Node>();
			children[5] = std::make_unique<Node>();
			children[6] = std::make_unique<Node>();
			children[7] = std::make_unique<Node>();

			DirectX::BoundingBox::CreateFromPoints(children[0]->bounds, { min.x, min.y, min.z, 0 }, { center.x, center.y, center.z, 0 });
			DirectX::BoundingBox::CreateFromPoints(children[1]->bounds, { center.x, min.y, min.z, 0 },	 { max.x, center.y, center.z, 0 });
			DirectX::BoundingBox::CreateFromPoints(children[2]->bounds, { min.x, min.y, center.z, 0 },	 { center.x, center.y, max.z, 0 });
			DirectX::BoundingBox::CreateFromPoints(children[3]->bounds, { center.x, min.y, center.z, 0 }, { max.x, center.y, max.z, 0 });
			DirectX::BoundingBox::CreateFromPoints(children[4]->bounds, { min.x, center.y, min.z, 0 },	 { center.x, max.y, center.z, 0 });
			DirectX::BoundingBox::CreateFromPoints(children[5]->bounds, { center.x, center.y, min.z, 0 }, { max.x, max.y, center.z, 0 });
			DirectX::BoundingBox::CreateFromPoints(children[6]->bounds, { min.x, center.y, center.z, 0 }, { center.x, max.y, max.z, 0 });
			DirectX::BoundingBox::CreateFromPoints(children[7]->bounds, { center.x, center.y, center.z, 0 }, { max.x, max.y, max.z, 0 });

			for (int i = 0; i < data.size(); i++)
				if (data[i] != nullptr)
				{
					DirectX::BoundingBox itemBounds;
					data[i]->StoreBounds(itemBounds);

					for (int j = 0; j < 8; j++)
						children[j]->Insert(data[i], itemBounds, depth + 1);
				}

			data.clear();
			isLeaf = false;
		}


		bool Insert(IEntity *item, const DirectX::BoundingBox &itemBounds, const UINT depth = 0)
		{
			if (!bounds.Intersects(itemBounds))
				return false;

			if (isLeaf)
			{
				if (depth >= _MAX_DEPTH || data.size() < _MAX_ITEMS_IN_NODE)
				{
					data.push_back(item);
					return true;
				}

				Split(depth);
			}

			for (int i = 0; i < 8; i++)
			{
				if (children[i] != nullptr)
					children[i]->Insert(item, itemBounds, depth + 1);
			}

			return true;
		}

		void Remove(IEntity *item, const DirectX::BoundingBox &itemBounds, const bool skipIntersection = false)
		{
			if (!skipIntersection)
				if (!bounds.Intersects(itemBounds))
					return;

			if (isLeaf)
			{
				std::erase_if(data, [item](const IEntity *otherItem) { return item == otherItem; });
				return;
			}

			for (int i = 0; i < 8; i++)
			{
				if (children[i] != nullptr)
					children[i]->Remove(item, itemBounds, skipIntersection);
			}

			bool gotHomogenousItems = false;
			std::vector<IEntity *> containingItems;

			for (int i = 0; i < 8; i++)
				if (children[i] != nullptr)
				{
					if (!children[i]->isLeaf)
						return;

					if (!children[i]->data.empty())
					{
						for (IEntity *childItem : children[i]->data)
						{
							if (childItem == nullptr)
								continue;

							if (std::ranges::find(containingItems, childItem) == containingItems.end())
								containingItems.push_back(childItem);
						}

						if (containingItems.size() > _MAX_ITEMS_IN_NODE)
							return;
					}
				}

			for (int i = 0; i < 8; i++)
			{
				children[i].release();
				children[i] = nullptr;
			}

			isLeaf = true;
			data.clear();
			for (IEntity *newItem : containingItems)
				data.push_back(newItem);
		}


		void AddToVector(std::vector<IEntity *> &containingItems) const
		{
			if (isLeaf)
			{
				for (IEntity *item : data)
				{
					if (item == nullptr)
						continue;

					if (std::ranges::find(containingItems, item) == containingItems.end())
						containingItems.push_back(item);
				}

				return;
			}

			for (int i = 0; i < 8; i++)
			{
				if (children[i] == nullptr)
					continue;

				children[i]->AddToVector(containingItems);
			}
		}

		void FrustumCull(const DirectX::BoundingFrustum &frustum, std::vector<IEntity *> &containingItems) const
		{
			switch (frustum.Contains(bounds))
			{
			case DISJOINT:
				return;

			case CONTAINS:
				AddToVector(containingItems);
				return;

			case INTERSECTS:
				if (isLeaf)
				{
					for (IEntity *item : data)
					{
						if (item == nullptr)
							continue;

						if (std::ranges::find(containingItems, item) == containingItems.end())
							containingItems.push_back(item);
					}

					return;
				}

				for (int i = 0; i < 8; i++)
				{
					if (children[i] == nullptr)
						continue;

					children[i]->FrustumCull(frustum, containingItems);
				}
				break;
			}
		}
	};

	std::unique_ptr<Node> _root;


public:
	Octree() = default;
	~Octree() = default;
	Octree(const Octree &other) = delete;
	Octree &operator=(const Octree &other) = delete;
	Octree(Octree &&other) = delete;
	Octree &operator=(Octree &&other) = delete;

	[[nodiscard]] bool Initialize(const DirectX::BoundingBox &sceneBounds)
	{
		_root = std::make_unique<Node>();
		_root->bounds = sceneBounds;

		return true;
	}

	void Insert(IEntity *data, const DirectX::BoundingBox &bounds) const
	{
		if (_root != nullptr)
			_root->Insert(data, bounds);
	}

	[[nodiscard]] bool Remove(IEntity *data, const DirectX::BoundingBox &bounds) const
	{
		if (_root == nullptr)
			return false;

		_root->Remove(data, bounds);
		return true;
	}

	[[nodiscard]] bool Remove(IEntity *data) const
	{
		if (_root == nullptr)
			return false;

		_root->Remove(data, _root->bounds, true);
		return true;
	}

	[[nodiscard]] bool FrustumCull(const DirectX::BoundingFrustum &frustum, std::vector<IEntity *> &containingItems) const
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