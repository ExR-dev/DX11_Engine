#include "SubMeshD3D11.h"

#include "ErrMsg.h"


bool SubMeshD3D11::Initialize(
	const size_t startIndexValue, const size_t nrOfIndicesInSubMesh,
	ID3D11ShaderResourceView *ambientTextureSRV, ID3D11ShaderResourceView *diffuseTextureSRV,
	ID3D11ShaderResourceView *specularTextureSRV)
{
	_startIndex = startIndexValue;
	_nrOfIndices = nrOfIndicesInSubMesh;

	_ambientTexture = ambientTextureSRV;
	_diffuseTexture = diffuseTextureSRV;
	_specularTexture = specularTextureSRV;

	return true;
}


bool SubMeshD3D11::PerformDrawCall(ID3D11DeviceContext *context) const
{
	context->DrawIndexed(_nrOfIndices, _startIndex, 0);
	return true;
}


ID3D11ShaderResourceView *SubMeshD3D11::GetAmbientSRV() const
{
	return _ambientTexture;
}

ID3D11ShaderResourceView *SubMeshD3D11::GetDiffuseSRV() const
{
	return _diffuseTexture;
}

ID3D11ShaderResourceView *SubMeshD3D11::GetSpecularSRV() const
{
	return _specularTexture;
}
