#include "MeshD3D11.h"

#include "../ErrMsg.h"


MeshD3D11::~MeshD3D11()
{
	
}


bool MeshD3D11::Initialize(ID3D11Device *device, const MeshData &meshInfo)
{
	if (!_vertexBuffer.Initialize(device, meshInfo.vertexInfo.sizeOfVertex, meshInfo.vertexInfo.nrOfVerticesInBuffer, meshInfo.vertexInfo.vertexData))
	{
		ErrMsg("Failed to initialize vertex buffer!");
		return false;
	}

	if (!_indexBuffer.Initialize(device, meshInfo.indexInfo.nrOfIndicesInBuffer, meshInfo.indexInfo.indexData))
	{
		ErrMsg("Failed to initialize index buffer!");
		return false;
	}

	const size_t subMeshCount = meshInfo.subMeshInfo.size();
	for (size_t i = 0; i < subMeshCount; i++)
	{
		SubMeshD3D11 subMesh;
		if (!subMesh.Initialize(meshInfo.subMeshInfo.at(i).startIndexValue, meshInfo.subMeshInfo.at(i).nrOfIndicesInSubMesh, nullptr, nullptr, nullptr)) // TODO
		{
			ErrMsg("Failed to initialize sub mesh!");
			return false;
		}

		_subMeshes.push_back(subMesh);
	}

	return true;
}


bool MeshD3D11::BindMeshBuffers(ID3D11DeviceContext *context) const
{
	// TODO: Bind submesh data

	ID3D11Buffer *const vertxBuffer = _vertexBuffer.GetBuffer();

	const UINT stride = _vertexBuffer.GetVertexSize();
	const UINT offset = 0;

	context->IASetVertexBuffers(0, 1, &vertxBuffer, &stride, &offset);
	context->IASetIndexBuffer(_indexBuffer.GetBuffer(), DXGI_FORMAT_R32_UINT, 0);

	return true;
}

bool MeshD3D11::PerformSubMeshDrawCall(ID3D11DeviceContext *context, const size_t subMeshIndex) const
{
	_subMeshes.at(subMeshIndex).PerformDrawCall(context);
	return true;
}


size_t MeshD3D11::GetNrOfSubMeshes() const
{
	return _subMeshes.size();
}

ID3D11ShaderResourceView *MeshD3D11::GetAmbientSRV(const size_t subMeshIndex) const
{
	if (GetNrOfSubMeshes() <= subMeshIndex)
		return nullptr;

	return _subMeshes.at(subMeshIndex).GetAmbientSRV();
}

ID3D11ShaderResourceView *MeshD3D11::GetDiffuseSRV(const size_t subMeshIndex) const
{
	if (GetNrOfSubMeshes() <= subMeshIndex)
		return nullptr;

	return _subMeshes.at(subMeshIndex).GetDiffuseSRV();
}

ID3D11ShaderResourceView *MeshD3D11::GetSpecularSRV(const size_t subMeshIndex) const
{
	if (GetNrOfSubMeshes() <= subMeshIndex)
		return nullptr;

	return _subMeshes.at(subMeshIndex).GetSpecularSRV();
}
