#include "SubMeshD3D11.h"

#include "ErrMsg.h"


bool SubMeshD3D11::Initialize(
	const size_t startIndexValue, const size_t nrOfIndicesInSubMesh, const std::string &materialName,
	ID3D11ShaderResourceView *ambientTextureSRV, ID3D11ShaderResourceView *diffuseTextureSRV, ID3D11ShaderResourceView *specularTextureSRV)
{
	_startIndex = startIndexValue;
	_nrOfIndices = nrOfIndicesInSubMesh;

	_materialName = materialName;

	return true;
}


bool SubMeshD3D11::PerformDrawCall(ID3D11DeviceContext *context) const
{
	context->DrawIndexed(static_cast<UINT>(_nrOfIndices), static_cast<UINT>(_startIndex), 0);
	return true;
}


const std::string &SubMeshD3D11::GetMaterialName() const
{
	return _materialName;
}
