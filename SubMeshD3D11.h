#pragma once

#include <string>
#include <d3d11_4.h>
#include <DirectXCollision.h>


class SubMeshD3D11
{
private:
	size_t _startIndex = 0;
	size_t _nrOfIndices = 0;

	DirectX::BoundingBox _boundingBox;

	ID3D11ShaderResourceView *_ambientTexture = nullptr;
	ID3D11ShaderResourceView *_diffuseTexture = nullptr;
	ID3D11ShaderResourceView *_specularTexture = nullptr;

public:
	SubMeshD3D11() = default;
	~SubMeshD3D11() = default;
	SubMeshD3D11(const SubMeshD3D11 &other) = default;
	SubMeshD3D11 &operator=(const SubMeshD3D11 &other) = default;
	SubMeshD3D11(SubMeshD3D11 &&other) = default;
	SubMeshD3D11 &operator=(SubMeshD3D11 &&other) = default;

	[[nodiscard]] bool Initialize(size_t startIndexValue, size_t nrOfIndicesInSubMesh, const DirectX::BoundingBox &boundingBox,
		ID3D11ShaderResourceView *ambientTextureSRV, ID3D11ShaderResourceView *diffuseTextureSRV, ID3D11ShaderResourceView *specularTextureSRV);

	[[nodiscard]] bool PerformDrawCall(ID3D11DeviceContext *context) const;

	[[nodiscard]] const DirectX::BoundingBox &GetBoundingBox() const;

	[[nodiscard]] ID3D11ShaderResourceView *GetAmbientSRV() const;
	[[nodiscard]] ID3D11ShaderResourceView *GetDiffuseSRV() const;
	[[nodiscard]] ID3D11ShaderResourceView *GetSpecularSRV() const;
};
