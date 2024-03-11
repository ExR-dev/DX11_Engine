#pragma once

#include <array>
#include <d3d11_4.h>

#include "CameraD3D11.h"


class Cubemap
{
private:
	std::array<CameraD3D11*, 6> _cameras;
	ShaderResourceTextureD3D11 _texture;

public:
	Cubemap();
	Cubemap(ID3D11Device *device, UINT resolution, float nearZ, float farZ, const DirectX::XMFLOAT4A &initialPosition);
	~Cubemap();
	Cubemap(const Cubemap &other) = delete;
	Cubemap &operator=(const Cubemap &other) = delete;
	Cubemap(Cubemap &&other) = delete;
	Cubemap &operator=(Cubemap &&other) = delete;

	[[nodiscard]] bool Initialize(ID3D11Device *device, UINT resolution, float nearZ, float farZ, const DirectX::XMFLOAT4A &initialPosition);

	[[nodiscard]] bool UpdateBuffers(ID3D11DeviceContext *context) const;

	[[nodiscard]] CameraD3D11 *GetCamera(UINT index) const;

};