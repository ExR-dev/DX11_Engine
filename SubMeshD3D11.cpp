#include "SubMeshD3D11.h"

#include "ErrMsg.h"


bool SubMeshD3D11::Initialize(
	const size_t startIndexValue, const size_t nrOfIndicesInSubMesh, const DirectX::BoundingBox &boundingBox,
	ID3D11ShaderResourceView *ambientTextureSRV, ID3D11ShaderResourceView *diffuseTextureSRV, ID3D11ShaderResourceView *specularTextureSRV)
{
	if (_ambientTexture != nullptr)
	{
		ErrMsg("Ambient texture is not nullptr!");
		return false;
	}
	if (_diffuseTexture != nullptr)
	{
		ErrMsg("Diffuse texture is not nullptr!");
		return false;
	}
	if (_specularTexture != nullptr)
	{
		ErrMsg("Specular texture is not nullptr!");
		return false;
	}

	_startIndex = startIndexValue;
	_nrOfIndices = nrOfIndicesInSubMesh;

	_boundingBox = boundingBox;

	_ambientTexture = ambientTextureSRV;
	_diffuseTexture = diffuseTextureSRV;
	_specularTexture = specularTextureSRV;

	ErrMsg(std::format("Submesh created with center: ({}, {}, {}), extents: ({}, {}, {}).", 
		boundingBox.Center.x, boundingBox.Center.y, boundingBox.Center.z,
		boundingBox.Extents.x, boundingBox.Extents.y, boundingBox.Extents.z
	));

	return true;
}


bool SubMeshD3D11::PerformDrawCall(ID3D11DeviceContext *context) const
{
	context->DrawIndexed(_nrOfIndices, _startIndex, 0);
	return true;
}


const DirectX::BoundingBox &SubMeshD3D11::GetBoundingBox() const
{
	return _boundingBox;
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
