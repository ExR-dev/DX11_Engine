#pragma once

#include <d3d11.h>

#include "Content.h"


struct RenderInstance
{
	UINT inputLayoutID = CONTENT_LOAD_ERROR;
	UINT meshID = CONTENT_LOAD_ERROR;
	UINT vsID = CONTENT_LOAD_ERROR;
	UINT psID = CONTENT_LOAD_ERROR;
	UINT texID = CONTENT_LOAD_ERROR;

	void *subject;
	size_t subjectSize;
};


class Graphics
{
private:
	IDXGISwapChain			*_swapChain;
	ID3D11RenderTargetView	*_rtv;
	ID3D11Texture2D			*_dsTexture;
	ID3D11DepthStencilView	*_dsView;
	D3D11_VIEWPORT			 _viewport;

	ID3D11DeviceContext		*_context;
	Content					*_content;

	bool _isSetup;
	bool _isRendering;

	std::vector<RenderInstance> _renderInstances;


	bool FlushRenderQueue();
	bool ResetRenderState();

public:
	Graphics();
	~Graphics();

	bool Setup(UINT width, UINT height, HWND window, 
		ID3D11Device *&device, ID3D11DeviceContext *&immediateContext, Content *content);

	[[nodiscard]] ID3D11RenderTargetView *GetRTV() const;
	[[nodiscard]] ID3D11Texture2D *GetDsTexture() const;
	[[nodiscard]] ID3D11DepthStencilView *GetDsView() const;
	[[nodiscard]] D3D11_VIEWPORT &GetViewport();

	bool BeginRender();
	bool QueueRender(const RenderInstance &instance);
	bool EndRender();
};
