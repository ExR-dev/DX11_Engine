#include "SubMeshD3D11.h"

#include "ErrMsg.h"


SubMeshD3D11::~SubMeshD3D11()
{
	if (_specularTextureSRV != nullptr)
		_specularTextureSRV->Release();

	if (_diffuseTextureSRV != nullptr)
		_diffuseTextureSRV->Release();

	if (_ambientTextureSRV != nullptr)
		_ambientTextureSRV->Release();
}

bool SubMeshD3D11::Initialize(
	const size_t startIndexValue, const size_t nrOfIndicesInSubMesh, const std::string &materialName,
	ID3D11ShaderResourceView *ambientTextureSRV, ID3D11ShaderResourceView *diffuseTextureSRV, ID3D11ShaderResourceView *specularTextureSRV)
{
	_startIndex = startIndexValue;
	_nrOfIndices = nrOfIndicesInSubMesh;

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
