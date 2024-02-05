#pragma once

#include <d3d11.h>

#include "Entity.h"


class Graphics
{
private:
	IDXGISwapChain			*_swapChain;
	ID3D11RenderTargetView	*_rtv;
	ID3D11Texture2D			*_dsTexture;
	ID3D11DepthStencilView	*_dsView;
	D3D11_VIEWPORT			 _viewport;

	bool _isSetup;
	bool _isRendering;

public:
	Graphics();
	~Graphics();

	bool Setup(
		UINT width, UINT height, HWND window, 
		ID3D11Device *&device, ID3D11DeviceContext *&immediateContext);

	bool BeginRender();
	bool EndRender();

	bool Render(const Entity &entity);
};
