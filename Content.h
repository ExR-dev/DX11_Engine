#pragma once

#include <vector>

#include "ToDistribute/MeshD3D11.h"


class Content
{
private:
	std::vector<MeshD3D11> _meshes;

public:
	Content();
	~Content();

};