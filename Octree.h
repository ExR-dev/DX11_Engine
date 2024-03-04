#pragma once

#include <memory>
#include <DirectXCollision.h>
#include <set>
#include <vector>


constexpr int MAX_ITEMS_IN_NODE = 8;
constexpr float MIN_NODE_SIZE = 4.0f;


template<typename T>
class Octree
{
private:
	struct Node
	{
		std::vector<T*> data;
		DirectX::BoundingBox bounds;
		std::unique_ptr<Node> children[8];
		bool isLeaf = true;

		
		[[nodiscard]] bool IsMinSize() const
		{
			return (bounds.Extents.x * 2.0f + (MIN_NODE_SIZE / 32.0f) <= MIN_NODE_SIZE);
		}


		void Split()
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
			{
				for (int j = 0; j < 8; j++)
				{
					if (data[i] != nullptr)
						children[i]->Insert(data[i], bounds);
				}
			}

			data.clear();
			isLeaf = false;
		}


		bool Insert(T *item, const DirectX::BoundingBox &itemBounds)
		{
			if (!bounds.Intersects(itemBounds))
				return false;

			if (isLeaf)
			{
				if (IsMinSize() || data.size() < MAX_ITEMS_IN_NODE)
				{
					data.push_back(item);
					return true;
				}

				Split();
			}

			for (int i = 0; i < 8; i++)
			{
				if (children[i] != nullptr)
					children[i]->Insert(item, itemBounds);
			}

			return true;
		}

		void Remove(T *item, const DirectX::BoundingBox &itemBounds)
		{
			if (!bounds.Intersects(itemBounds))
				return;

			if (isLeaf)
			{
				std::erase_if(data, [item](const T *otherItem) { return item == otherItem; });
				return;
			}

			for (int i = 0; i < 8; i++)
			{
				if (children[i] != nullptr)
					children[i]->Remove(item, itemBounds);
			}
		}


		void AddToSet(std::set<T*> &containingItems)
		{
			if (isLeaf)
			{
				for (T *item : data)
				{
					if (item != nullptr)
						containingItems.insert(item);
				}

				return;
			}

			for (int i = 0; i < 8; i++)
			{
				if (children[i] != nullptr)
					children[i]->AddToSet(containingItems);
			}
		}

		void FrustumCull(const DirectX::BoundingFrustum &frustum, std::set<T *> &containingItems)
		{
			if (!frustum.Intersects(bounds))
				return;

			if (frustum.Contains(bounds))
			{
				AddToSet(containingItems);
				return;
			}

			if (isLeaf)
			{
				for (T *item : data)
				{
					if (item != nullptr)
						containingItems.insert(item);
				}

				return;
			}

			for (int i = 0; i < 8; i++)
			{
				if (children[i] != nullptr)
					children[i]->FrustumCull(frustum, containingItems);
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

	void Insert(T *data, const DirectX::BoundingBox &bounds)
	{
		if (_root != nullptr)
			_root->Insert(data, bounds);
	}

	[[nodiscard]] bool Remove(T *data, const DirectX::BoundingBox &bounds)
	{
		if (_root == nullptr)
			return false;

		_root->Remove(data, bounds);
		return true;
	}

	[[nodiscard]] bool Remove(T *data)
	{
		if (_root == nullptr)
			return false;

		_root->Remove(data, _root->bounds);
		return true;
	}

	[[nodiscard]] bool FrustumCull(const DirectX::BoundingFrustum &frustum, std::set<T*> &containingItems)
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