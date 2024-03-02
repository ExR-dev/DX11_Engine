#pragma once

#include <memory>
#include <DirectXCollision.h>


template<typename T>
class Octree
{
private:
	struct Node
	{
		T data;
		DirectX::BoundingBox bounds;
		std::unique_ptr<Node> children[8];
	};

	std::unique_ptr<Node> _root;


public:
	Octree() = default;
	~Octree() = default;
	Octree(const Octree &other) = delete;
	Octree &operator=(const Octree &other) = delete;
	Octree(Octree &&other) = delete;
	Octree &operator=(Octree &&other) = delete;

	[[nodiscard]] bool Initialize();
	[[nodiscard]] bool Update();
	[[nodiscard]] bool Render();
};