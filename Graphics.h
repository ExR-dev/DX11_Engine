#pragma once

#include <d3d11.h>

#include "CameraD3D11.h"
#include "Content.h"


struct RenderInstance
{
	UINT
		meshID = CONTENT_LOAD_ERROR,
		vsID = CONTENT_LOAD_ERROR,
		psID = CONTENT_LOAD_ERROR,
		texID = CONTENT_LOAD_ERROR,
		samplerID = CONTENT_LOAD_ERROR,
		inputLayoutID = CONTENT_LOAD_ERROR;

	void *subject;
	size_t subjectSize;
};


class Graphics
{
private:
	ID3D11DeviceContext		*_context;
	Content					*_content;

	bool _isSetup;
	bool _isRendering;

	IDXGISwapChain			*_swapChain;
	ID3D11RenderTargetView	*_rtv;
	ID3D11Texture2D			*_dsTexture;
	ID3D11DepthStencilView	*_dsView;
	D3D11_VIEWPORT			 _viewport;

	CameraD3D11 *_currCamera = nullptr;
	std::vector<RenderInstance> _renderInstances;

	UINT
		_currMeshID			= CONTENT_LOAD_ERROR,
		_currVsID			= CONTENT_LOAD_ERROR,
		_currPsID			= CONTENT_LOAD_ERROR,
		_currTexID			= CONTENT_LOAD_ERROR,
		_currSamplerID		= CONTENT_LOAD_ERROR,
		_currInputLayoutID	= CONTENT_LOAD_ERROR;


	[[nodiscard]] bool FlushRenderQueue();
	[[nodiscard]] bool ResetRenderState();

public:
	Graphics();
	~Graphics();

	[[nodiscard]] bool Setup(UINT width, UINT height, HWND window,
		ID3D11Device *&device, ID3D11DeviceContext *&immediateContext, Content *content);

	[[nodiscard]] ID3D11RenderTargetView *GetRTV() const;
	[[nodiscard]] ID3D11Texture2D *GetDsTexture() const;
	[[nodiscard]] ID3D11DepthStencilView *GetDsView() const;
	[[nodiscard]] D3D11_VIEWPORT &GetViewport();

	[[nodiscard]] bool SetCamera(CameraD3D11 *camera);

	[[nodiscard]] bool BeginRender();
	[[nodiscard]] bool QueueRender(const RenderInstance &instance);
	[[nodiscard]] bool EndRender();
};
