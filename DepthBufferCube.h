#pragma once

#include <d3d11_4.h>

#include <vector>
#include <array>


class DepthBufferCube
{
private:
	ID3D11Texture2D *_texture = nullptr;
	std::vector<std::array<ID3D11DepthStencilView *, 6>> _dsvs;
	ID3D11ShaderResourceView *_srv = nullptr;

public:
	DepthBufferCube() = default;
	DepthBufferCube(ID3D11Device *device, UINT width, UINT height);
	~DepthBufferCube();
	DepthBufferCube(const DepthBufferCube &other) = delete;
	DepthBufferCube &operator=(const DepthBufferCube &other) = delete;
	DepthBufferCube(DepthBufferCube &&other) = delete;
	DepthBufferCube &operator=(DepthBufferCube &&other) = delete;

	[[nodiscard]] bool Initialize(ID3D11Device *device, UINT width, UINT height, UINT arraySize = 1);

	[[nodiscard]] ID3D11DepthStencilView *GetDSV(UINT arrayIndex, UINT viewIndex) const;
	[[nodiscard]] ID3D11ShaderResourceView *GetSRV() const;
};