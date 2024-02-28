#pragma once

#include <d3d11_4.h>
#include <vector>
#include <DirectXCollision.h>

#include "SubMeshD3D11.h"
#include "VertexBufferD3D11.h"
#include "IndexBufferD3D11.h"


struct MeshData
{
	struct VertexInfo
	{
		size_t sizeOfVertex = 0;
		size_t nrOfVerticesInBuffer = 0;
		float *vertexData = nullptr;

		~VertexInfo()
		{
			delete[] vertexData;
		}
	} vertexInfo;

	struct IndexInfo
	{
		size_t nrOfIndicesInBuffer = 0;
		uint32_t *indexData = nullptr;

		~IndexInfo()
		{
			delete[] indexData;
		}
	} indexInfo;

	struct SubMeshInfo
	{
		size_t startIndexValue = 0;
		size_t nrOfIndicesInSubMesh = 0;
		ID3D11ShaderResourceView *ambientTextureSRV = nullptr;
		ID3D11ShaderResourceView *diffuseTextureSRV = nullptr;
		ID3D11ShaderResourceView *specularTextureSRV = nullptr;
		DirectX::BoundingBox boundingBox;
	};

	std::vector<SubMeshInfo> subMeshInfo;
};

class MeshD3D11
{
private:
	std::vector<SubMeshD3D11> _subMeshes;
	VertexBufferD3D11 _vertexBuffer;
	IndexBufferD3D11 _indexBuffer;


public:
	MeshD3D11() = default;
	~MeshD3D11();
	MeshD3D11(const MeshD3D11 &other) = delete;
	MeshD3D11 &operator=(const MeshD3D11 &other) = delete;
	MeshD3D11(MeshD3D11 &&other) = delete;
	MeshD3D11 &operator=(MeshD3D11 &&other) = delete;

	[[nodiscard]] bool Initialize(ID3D11Device *device, const MeshData &meshInfo);

	[[nodiscard]] bool BindMeshBuffers(ID3D11DeviceContext *context, UINT stride = 0, UINT offset = 0) const;
	[[nodiscard]] bool PerformSubMeshDrawCall(ID3D11DeviceContext *context, size_t subMeshIndex) const;

	[[nodiscard]] size_t GetNrOfSubMeshes() const;
	[[nodiscard]] ID3D11ShaderResourceView *GetAmbientSRV(size_t subMeshIndex) const;
	[[nodiscard]] ID3D11ShaderResourceView *GetDiffuseSRV(size_t subMeshIndex) const;
	[[nodiscard]] ID3D11ShaderResourceView *GetSpecularSRV(size_t subMeshIndex) const;
};