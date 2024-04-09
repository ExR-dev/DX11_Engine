#pragma once

#include <memory>
#include <utility>
#include <vector>
#include <DirectXCollision.h>

#include "Entity.h"
#include "Raycast.h"


class Octree
{
private:
	static constexpr UINT MAX_ITEMS_IN_NODE = 16;
	static constexpr UINT MAX_DEPTH = 5;


	struct Node
	{
		std::vector<Entity *> data;
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
			DirectX::BoundingBox::CreateFromPoints(children[1]->bounds, { center.x, min.y, min.z, 0 }, { max.x, center.y, center.z, 0 });
			DirectX::BoundingBox::CreateFromPoints(children[2]->bounds, { min.x, min.y, center.z, 0 }, { center.x, center.y, max.z, 0 });
			DirectX::BoundingBox::CreateFromPoints(children[3]->bounds, { center.x, min.y, center.z, 0 }, { max.x, center.y, max.z, 0 });
			DirectX::BoundingBox::CreateFromPoints(children[4]->bounds, { min.x, center.y, min.z, 0 }, { center.x, max.y, center.z, 0 });
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

			for (int i = 0; i < 8; i++)
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

			for (int i = 0; i < 8; i++)
			{
				if (children[i] != nullptr)
					children[i]->Remove(item, itemBounds, depth + 1, skipIntersection);
			}

			std::vector<Entity *> containingItems;
			containingItems.reserve(MAX_ITEMS_IN_NODE);
			for (int i = 0; i < 8; i++)
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

			for (int i = 0; i < 8; i++)
			{
				children[i].release();
				children[i] = nullptr;
			}

			isLeaf = true;
			data.clear();
			for (Entity *newItem : containingItems)
				data.push_back(newItem);
		}


		void AddToVector(std::vector<Entity *> &containingItems, const UINT depth) const
		{
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

			for (int i = 0; i < 8; i++)
			{
				if (children[i] == nullptr)
					continue;

				children[i]->AddToVector(containingItems, depth + 1);
			}
		}

		void FrustumCull(const DirectX::BoundingFrustum &frustum, std::vector<Entity *> &containingItems, const UINT depth = 0) const
		{
			switch (frustum.Contains(bounds))
			{
				case DirectX::DISJOINT:
					return;

				case DirectX::CONTAINS:
					AddToVector(containingItems, depth + 1);
					break;

				case DirectX::INTERSECTS:
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

					for (int i = 0; i < 8; i++)
					{
						if (children[i] == nullptr)
							continue;

						children[i]->FrustumCull(frustum, containingItems, depth + 1);
					}
					break;
			}
		}

		void BoxCull(const DirectX::BoundingOrientedBox &box, std::vector<Entity *> &containingItems, const UINT depth = 0) const
		{
			switch (box.Contains(bounds))
			{
				case DirectX::DISJOINT:
					return;

				case DirectX::CONTAINS:
					AddToVector(containingItems, depth + 1);
					break;

				case DirectX::INTERSECTS:
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

					for (int i = 0; i < 8; i++)
					{
						if (children[i] == nullptr)
							continue;

						children[i]->BoxCull(box, containingItems, depth + 1);
					}
					break;
			}
		}


		bool Raycast(const DirectX::XMFLOAT3A &orig, const DirectX::XMFLOAT3A &dir, float &length, Entity *&entity) const
		{
			if (isLeaf)
			{ // Check all items in leaf for intersection & return result.
				for (Entity *item : data)
				{
					if (item == nullptr)
						continue;

					DirectX::BoundingBox itemBounds;
					item->StoreBounds(itemBounds);

					float newLength = 0.0f;
					if (itemBounds.Intersects(TO_CONST_VEC(orig), TO_CONST_VEC(dir), newLength))
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

			struct ChildHit { int index; float length; };
			std::vector<ChildHit> childHits = {
				{ 0, FLT_MAX }, { 1, FLT_MAX }, { 2, FLT_MAX }, { 3, FLT_MAX },
				{ 4, FLT_MAX }, { 5, FLT_MAX }, { 6, FLT_MAX }, { 7, FLT_MAX }
			};

			int childHitCount = 8;
			for (int i = 0; i < childHitCount; i++)
			{
				const Node *child = children[childHits[i].index].get();
				if (child->bounds.Intersects(TO_CONST_VEC(orig), TO_CONST_VEC(dir), childHits[i].length))
					continue;

				// Remove child node from hits.
				childHits.erase(childHits.begin() + i);
				childHitCount--;
				i--;
			}

			// Insertion sort by length.
			for (int i = 1; i < childHitCount; i++)
			{
				int j = i;
				while (childHits[j].length < childHits[j-1].length)
				{
					std::swap(childHits[j], childHits[j-1]);
					if (--j <= 0) 
						break;
				}
			}

			// Check children in order of closest to furthest. Return first intersection found.
			for (int i = 0; i < childHitCount; i++)
			{
				length = FLT_MAX;
				entity = nullptr;

				if (children[childHits[i].index]->Raycast(orig, dir, length, entity))
					return true;
			}

			return false;
		}


		void DebugGetStructure(std::vector<DirectX::BoundingBox> &boxCollection) const
		{
			if (isLeaf)
			{
				boxCollection.push_back(bounds);
				return;
			}

			for (int i = 0; i < 8; i++)
				children[i]->DebugGetStructure(boxCollection);
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

	[[nodiscard]] bool BoxCull(const DirectX::BoundingOrientedBox &box, std::vector<Entity *> &containingItems) const
	{
		if (_root == nullptr)
			return false;

		_root->BoxCull(box, containingItems);
		return true;
	}


	bool Raycast(const DirectX::XMFLOAT3A &orig, const DirectX::XMFLOAT3A &dir, float &length, Entity *&entity) const
	{
		if (_root == nullptr)
			return false;

		float _ = FLT_MAX; // In case Intersects() uses the initial dist value as a maximum. Docs don't specify.
		if (!_root->bounds.Intersects(TO_CONST_VEC(orig), TO_CONST_VEC(dir), _))
			return false;

		length = FLT_MAX;
		entity = nullptr;

		return _root->Raycast(orig, dir, length, entity);
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