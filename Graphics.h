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

	ID3D11DeviceContext		*_context;

	bool _isSetup;
	bool _isRendering;

	UINT _currInputLayoutID = CONTENT_LOAD_ERROR;
	UINT _currMeshID = CONTENT_LOAD_ERROR;
	UINT _currVsID = CONTENT_LOAD_ERROR;
	UINT _currPsID = CONTENT_LOAD_ERROR;
	UINT _currTexID = CONTENT_LOAD_ERROR;

public:
	Graphics();
	~Graphics();

	bool Setup(
		UINT width, UINT height, HWND window, 
		ID3D11Device *&device, ID3D11DeviceContext *&immediateContext);

	ID3D11RenderTargetView *GetRTV();
	ID3D11Texture2D *GetDsTexture();
	ID3D11DepthStencilView *GetDsView();
	D3D11_VIEWPORT &GetViewport();

	bool BeginRender(const Content &content);
	bool EndRender();

	bool Render(Entity &entity, const Content &content);
};
