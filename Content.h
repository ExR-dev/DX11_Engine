#pragma once

#include <vector>

//#include "ToDistribute/InputLayoutD3D11.h"
//#include "ToDistribute/RenderTargetD3D11.h"
//
//#include "ToDistribute/ConstantBufferD3D11.h"
//#include "ToDistribute/DepthBufferD3D11.h"
//#include "ToDistribute/StructuredBufferD3D11.h"
//
#include "ToDistribute/ShaderD3D11.h"
//#include "ToDistribute/SamplerD3D11.h"
//#include "ToDistribute/ShaderResourceTextureD3D11.h"
//
//#include "ToDistribute/SpotLightCollectionD3D11.h"
//
#include "ToDistribute/MeshD3D11.h"
//#include "ToDistribute/SubMeshD3D11.h"
//
//#include "ToDistribute/VertexBufferD3D11.h"
//#include "ToDistribute/IndexBufferD3D11.h"


class Content
{
private:
	std::vector<MeshD3D11> _meshes;

	//std::vector<ShaderD3D11> _shaders;

public:
	Content();
	~Content();
	Content(const Content &other) = delete;
	Content &operator=(const Content &other) = delete;
	Content(Content &&other) = delete;
	Content &operator=(Content &&other) = delete;
};