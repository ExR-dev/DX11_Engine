#pragma once

#include <d3d11_4.h>


class RenderTargetD3D11
{
private:
	ID3D11Texture2D *_texture = nullptr;
	ID3D11RenderTargetView *_rtv = nullptr;
	ID3D11ShaderResourceView *_srv = nullptr;

public:
	RenderTargetD3D11() = default;
	~RenderTargetD3D11();
	RenderTargetD3D11(const RenderTargetD3D11 &other) = delete;
	RenderTargetD3D11 &operator=(const RenderTargetD3D11 &other) = delete;
	RenderTargetD3D11(RenderTargetD3D11 &&other) = delete;
	RenderTargetD3D11 &operator=(RenderTargetD3D11 &&other) = delete;

	[[nodiscard]] bool Initialize(ID3D11Device *device, UINT width, UINT height,
		DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM, bool hasSRV = false);

	[[nodiscard]] ID3D11RenderTargetView *GetRTV() const;
	[[nodiscard]] ID3D11ShaderResourceView *GetSRV() const;
};