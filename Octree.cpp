#include "Octree.h"

#include "ErrMsg.h"


template <typename T>
Octree<T>::Octree(const DirectX::BoundingBox &bounds)
{
	_root = std::make_unique<Node>(DirectX::BoundingBox(bounds));
}


template <typename T>
void Octree<T>::Insert(const T *data, const DirectX::BoundingBox &bounds)
{

}

