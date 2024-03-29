#pragma once

#include <array>
#include <d3d11_4.h>

#include "CameraD3D11.h"
#include "RenderTargetD3D11.h"
#include "Time.h"


constexpr UINT G_BUFFER_COUNT = 3;


class Cubemap
{
private:
	std::array<CameraD3D11 *, 6>					_cameras;
	std::array<ID3D11RenderTargetView *, 6>			_rtvs;
	std::array<ID3D11UnorderedAccessView *, 6>		_uavs;
	std::array<RenderTargetD3D11, G_BUFFER_COUNT>	_gBuffers;

	ID3D11Texture2D				*_dsTexture = nullptr;
	ID3D11DepthStencilView		*_dsView = nullptr;
	D3D11_VIEWPORT				_viewport = { };

	ShaderResourceTextureD3D11	_texture;

	float _updateTimer = 9999.9f;
	bool _doUpdate = true;


public:
	Cubemap();
	Cubemap(ID3D11Device *device, UINT resolution, float nearZ, float farZ, const DirectX::XMFLOAT4A &initialPosition);
	~Cubemap();
	Cubemap(const Cubemap &other) = delete;
	Cubemap &operator=(const Cubemap &other) = delete;
	Cubemap(Cubemap &&other) = delete;
	Cubemap &operator=(Cubemap &&other) = delete;

	[[nodiscard]] bool Initialize(ID3D11Device *device, UINT resolution, float nearZ, float farZ, const DirectX::XMFLOAT4A &initialPosition);

	[[nodiscard]] bool Update(ID3D11DeviceContext *context, Time &time);
	[[nodiscard]] bool UpdateBuffers(ID3D11DeviceContext *context) const;

	[[nodiscard]] bool GetUpdate() const;
	[[nodiscard]] CameraD3D11 *GetCamera(UINT index) const;
	[[nodiscard]] const std::array<RenderTargetD3D11, G_BUFFER_COUNT> *GetGBuffers() const;
	[[nodiscard]] ID3D11RenderTargetView *GetRTV(UINT index) const;
	[[nodiscard]] ID3D11UnorderedAccessView *GetUAV(UINT index) const;
	[[nodiscard]] ID3D11ShaderResourceView *GetSRV() const;
	[[nodiscard]] ID3D11DepthStencilView *GetDSV() const;
	[[nodiscard]] const D3D11_VIEWPORT &GetViewport() const;

};
