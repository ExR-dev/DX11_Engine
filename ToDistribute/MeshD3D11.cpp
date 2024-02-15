#include "MeshD3D11.h"


bool MeshD3D11::Initialize(ID3D11Device *device, const MeshData &meshInfo)
{
	if (!_vertexBuffer.Initialize(device, meshInfo.vertexInfo.sizeOfVertex, meshInfo.vertexInfo.nrOfVerticesInBuffer, meshInfo.vertexInfo.vertexData))
	{
		OutputDebugString(L"Failed to initialize vertex buffer!\n");
		return false;
	}

	if (!_indexBuffer.Initialize(device, meshInfo.indexInfo.nrOfIndicesInBuffer, meshInfo.indexInfo.indexData))
	{
		OutputDebugString(L"Failed to initialize index buffer!\n");
		return false;
	}

	int subMeshCount = meshInfo.subMeshInfo.size();
	for (int i = 0; i < subMeshCount; i++)
	{
		SubMeshD3D11 subMesh;
		if (!subMesh.Initialize(device, meshInfo.subMeshInfo.at(i)))
		{
			OutputDebugString(L"Failed to initialize sub mesh!\n");
			return false;
		}

		_subMeshes.push_back(subMesh);
	}

	return true;
}


bool MeshD3D11::BindMeshBuffers(ID3D11DeviceContext *context) const
{
	int subMeshCount = GetNrOfSubMeshes();

	for (int i = 0; i < subMeshCount; i++)
	{
		// TODO
		//const SubMeshD3D11 *subMesh = &_subMeshes.at(i);

		//context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

		//context->PSSetShaderResources(0, 1, &(subMesh.GetAmbientSRV()));
		//context->PSSetShaderResources(0, 1, &(subMesh.GetDiffuseSRV()));
		//context->PSSetShaderResources(0, 1, &(subMesh.GetSpecularSRV()));

	}

	// TODO
	ID3D11Buffer *const vertxBuffer = _vertexBuffer.GetBuffer();
	//context->IASetVertexBuffers(0, 1, &vertxBuffer, _vertexBuffer., &offset);

	return false;
}

bool MeshD3D11::PerformSubMeshDrawCall(ID3D11DeviceContext *context, size_t subMeshIndex) const
{
	// TODO
	return false;
}


size_t MeshD3D11::GetNrOfSubMeshes() const
{
	return _subMeshes.size();
}

ID3D11ShaderResourceView *MeshD3D11::GetAmbientSRV(size_t subMeshIndex) const
{
	if (GetNrOfSubMeshes() <= subMeshIndex)
		return nullptr;

	return _subMeshes.at(subMeshIndex).GetAmbientSRV();
}

ID3D11ShaderResourceView *MeshD3D11::GetDiffuseSRV(size_t subMeshIndex) const
{
	if (GetNrOfSubMeshes() <= subMeshIndex)
		return nullptr;

	return _subMeshes.at(subMeshIndex).GetDiffuseSRV();
}

ID3D11ShaderResourceView *MeshD3D11::GetSpecularSRV(size_t subMeshIndex) const
{
	if (GetNrOfSubMeshes() <= subMeshIndex)
		return nullptr;

	return _subMeshes.at(subMeshIndex).GetSpecularSRV();
}
