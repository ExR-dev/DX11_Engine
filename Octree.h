#pragma once

#include <memory>
#include <DirectXCollision.h>


#define MAX_ITEMS_IN_NODE 8


template<typename T>
class Octree
{
private:
	struct Node
	{
		std::array<T*, MAX_ITEMS_IN_NODE> data;
		DirectX::BoundingBox bounds;
		std::unique_ptr<Node> children[8];

		bool IsLeaf() const
		{
			return children[0] == nullptr;
		}

		void Split()
		{
			DirectX::XMFLOAT3
				center = bounds.Center,
				min = { center.x - bounds.Extents.x, center.y - bounds.Extents.y, center.z - bounds.Extents.z },
				max = { center.x + bounds.Extents.x, center.y + bounds.Extents.y, center.z + bounds.Extents.z };

			children[0] = std::make_unique<Node>(DirectX::BoundingBox(DirectX::XMFLOAT3(min.x, min.y, min.z), DirectX::XMFLOAT3(center.x, center.y, center.z)));
			children[1] = std::make_unique<Node>(DirectX::BoundingBox(DirectX::XMFLOAT3(center.x, min.y, min.z), DirectX::XMFLOAT3(max.x, center.y, center.z)));
			children[2] = std::make_unique<Node>(DirectX::BoundingBox(DirectX::XMFLOAT3(min.x, min.y, center.z), DirectX::XMFLOAT3(center.x, center.y, max.z)));
			children[3] = std::make_unique<Node>(DirectX::BoundingBox(DirectX::XMFLOAT3(center.x, min.y, center.z), DirectX::XMFLOAT3(max.x, center.y, max.z)));
			children[4] = std::make_unique<Node>(DirectX::BoundingBox(DirectX::XMFLOAT3(min.x, center.y, min.z), DirectX::XMFLOAT3(center.x, max.y, center.z)));
			children[5] = std::make_unique<Node>(DirectX::BoundingBox(DirectX::XMFLOAT3(center.x, center.y, min.z), DirectX::XMFLOAT3(max.x, max.y, center.z)));
			children[6] = std::make_unique<Node>(DirectX::BoundingBox(DirectX::XMFLOAT3(min.x, center.y, center.z), DirectX::XMFLOAT3(center.x, max.y, max.z)));
			children[7] = std::make_unique<Node>(DirectX::BoundingBox(DirectX::XMFLOAT3(center.x, center.y, center.z), DirectX::XMFLOAT3(max.x, max.y, max.z)));
			for (int i = 0; i < MAX_ITEMS_IN_NODE; ++i)
			{
				for (Node &child : children)
				{
					if (child->Insert(data[i], bounds))
					{
						data[i] = nullptr;
						break;
					}
				}
			}
		}

		bool Insert(const T *item, const DirectX::BoundingBox &itemBounds)
		{
			if (!bounds.Intersects(itemBounds))
				return false;

			if (IsLeaf())
			{
				if (data.size() < MAX_ITEMS_IN_NODE)
				{
					data.push_back(item);
					return true;
				}

				Split();
			}

			for (Node &child : children)
				if (child->Insert(item, itemBounds))
					return true;

			return false;
		}
	};

	std::unique_ptr<Node> _root;


public:
	Octree(const DirectX::BoundingBox &bounds);
	~Octree() = default;
	Octree(const Octree &other) = delete;
	Octree &operator=(const Octree &other) = delete;
	Octree(Octree &&other) = delete;
	Octree &operator=(Octree &&other) = delete;

	void Insert(const T *data, const DirectX::BoundingBox &bounds);


};