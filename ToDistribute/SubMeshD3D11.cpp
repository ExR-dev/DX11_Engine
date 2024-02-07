#include "SubMeshD3D11.h"


void SubMeshD3D11::Initialize(size_t startIndexValue, size_t nrOfIndicesInSubMesh,
	ID3D11ShaderResourceView *ambientTextureSRV, ID3D11ShaderResourceView *diffuseTextureSRV,
	ID3D11ShaderResourceView *specularTextureSRV)
{
	_startIndex = startIndexValue;
	_nrOfIndices = nrOfIndicesInSubMesh;

	_ambientTexture = ambientTextureSRV;
	_diffuseTexture = diffuseTextureSRV;
	_specularTexture = specularTextureSRV;
}


void SubMeshD3D11::PerformDrawCall(ID3D11DeviceContext *context) const
{
	// TODO
	context->DrawIndexed(_nrOfIndices, _startIndex, 0);
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
