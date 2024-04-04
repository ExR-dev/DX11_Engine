#pragma once

#include <string>
#include <d3d11_4.h>


class SubMeshD3D11
{
private:
	size_t _startIndex = 0;
	size_t _nrOfIndices = 0;

	std::string _materialName;

public:
	SubMeshD3D11() = default;
	~SubMeshD3D11() = default;
	SubMeshD3D11(const SubMeshD3D11 &other) = default;
	SubMeshD3D11 &operator=(const SubMeshD3D11 &other) = default;
	SubMeshD3D11(SubMeshD3D11 &&other) = default;
	SubMeshD3D11 &operator=(SubMeshD3D11 &&other) = default;

	[[nodiscard]] bool Initialize(size_t startIndexValue, size_t nrOfIndicesInSubMesh, const std::string &materialName,
		ID3D11ShaderResourceView *ambientTextureSRV, ID3D11ShaderResourceView *diffuseTextureSRV, ID3D11ShaderResourceView *specularTextureSRV);

	[[nodiscard]] bool PerformDrawCall(ID3D11DeviceContext *context) const;

	[[nodiscard]] const std::string &GetMaterialName() const;
};
