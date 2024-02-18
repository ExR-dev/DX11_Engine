#pragma once

#include <string>
#include <d3d11_4.h>


class SubMeshD3D11
{
private:
	size_t _startIndex = 0;
	size_t _nrOfIndices = 0;

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

	bool Initialize(size_t startIndexValue, size_t nrOfIndicesInSubMesh,
		ID3D11ShaderResourceView *ambientTextureSRV, ID3D11ShaderResourceView *diffuseTextureSRV,
		ID3D11ShaderResourceView *specularTextureSRV);

	bool PerformDrawCall(ID3D11DeviceContext *context) const;

	[[nodiscard]] ID3D11ShaderResourceView *GetAmbientSRV() const;
	[[nodiscard]] ID3D11ShaderResourceView *GetDiffuseSRV() const;
	[[nodiscard]] ID3D11ShaderResourceView *GetSpecularSRV() const;
};