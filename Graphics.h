#pragma once

#include <d3d11.h>
#include <array>

#include "Content.h"
#include "Time.h"
#include "Cubemap.h"
#include "RenderTargetD3D11.h"
#include "CameraD3D11.h"
#include "SpotLightCollectionD3D11.h"
#include "PointLightCollectionD3D11.h"


//constexpr UINT G_BUFFER_COUNT = 4;


class Graphics
{
private:
	bool _isSetup		= false;
	bool _isRendering	= false;

	ID3D11DeviceContext	*_context	= nullptr;
	Content	*_content				= nullptr;

	IDXGISwapChain *_swapChain		= nullptr;
	ID3D11RenderTargetView *_rtv	= nullptr;
	ID3D11Texture2D	*_dsTexture		= nullptr;
	ID3D11DepthStencilView *_dsView	= nullptr;
	ID3D11UnorderedAccessView *_uav	= nullptr;
	ID3D11BlendState *_tbs			= nullptr;
	ID3D11DepthStencilState *_ndss	= nullptr;
	ID3D11DepthStencilState *_tdss	= nullptr;
	D3D11_VIEWPORT _viewport		= { };

	ID3D11RasterizerState *_defaultRasterizer = nullptr;
	ID3D11RasterizerState *_wireframeRasterizer = nullptr;
	ID3D11RasterizerState *_shadowRasterizer = nullptr;
	bool _wireframe = false;

	std::array<RenderTargetD3D11, G_BUFFER_COUNT> _gBuffers;
	UINT _renderOutput = 0;

	CameraD3D11
		*_currMainCamera = nullptr,
		*_currViewCamera = nullptr;

	Cubemap *_currCubemap = nullptr;
	bool _updateCubemap = true;

	ConstantBufferD3D11 _globalLightBuffer;
	SpotLightCollectionD3D11 *_currSpotLightCollection = nullptr;
	PointLightCollectionD3D11 *_currPointLightCollection = nullptr;

	UINT
		_currMeshID			= CONTENT_LOAD_ERROR,
		_currVsID			= CONTENT_LOAD_ERROR,
		_currPsID			= CONTENT_LOAD_ERROR,
		_currTexID			= CONTENT_LOAD_ERROR,
		_currNormalID		= CONTENT_LOAD_ERROR,
		_currSpecularID		= CONTENT_LOAD_ERROR,
		_currReflectiveID	= CONTENT_LOAD_ERROR,
		_currAmbientID		= CONTENT_LOAD_ERROR,
		_currHeightID		= CONTENT_LOAD_ERROR,
		_currSamplerID		= CONTENT_LOAD_ERROR,
		_currInputLayoutID	= CONTENT_LOAD_ERROR;

	DirectX::XMFLOAT4A _ambientColor = { 0.07f, 0.075f, 0.08f, 0.0f };
	bool _renderTransparency = true;


	[[nodiscard]] bool RenderToTarget(
		const std::array<RenderTargetD3D11, G_BUFFER_COUNT> *targetGBuffers,
		ID3D11RenderTargetView *targetRTV,
		ID3D11UnorderedAccessView *targetUAV,
		ID3D11DepthStencilView *targetDSV, 
		const D3D11_VIEWPORT *targetViewport,
		bool renderGBuffer,
		bool cubemapStage
	);

	[[nodiscard]] bool RenderSpotlights();
	[[nodiscard]] bool RenderPointlights();
	[[nodiscard]] bool RenderShadowCasters();

	[[nodiscard]] bool RenderGeometry(const std::array<RenderTargetD3D11, G_BUFFER_COUNT> *targetGBuffers, 
		ID3D11DepthStencilView *targetDSV, const D3D11_VIEWPORT *targetViewport);
	[[nodiscard]] bool RenderLighting(const std::array<RenderTargetD3D11, G_BUFFER_COUNT> *targetGBuffers, 
		ID3D11UnorderedAccessView *targetUAV, const D3D11_VIEWPORT *targetViewport, bool useCubemapShader) const;
	[[nodiscard]] bool RenderGBuffer(UINT bufferIndex) const;
	[[nodiscard]] bool RenderTransparency(ID3D11RenderTargetView *targetRTV, ID3D11DepthStencilView *targetDSV, const D3D11_VIEWPORT *targetViewport);

	[[nodiscard]] bool ResetRenderState();


public:
	~Graphics();

	[[nodiscard]] bool Setup(UINT width, UINT height, HWND window,
		ID3D11Device *&device, ID3D11DeviceContext *&immediateContext, Content *content);

	[[nodiscard]] bool GetUpdateCubemap() const;

	[[nodiscard]] bool SetCameras(CameraD3D11 *mainCamera, CameraD3D11 *viewCamera = nullptr);
	[[nodiscard]] bool SetCubemap(Cubemap *cubemap);
	[[nodiscard]] bool SetSpotlightCollection(SpotLightCollectionD3D11 *spotlights);
	[[nodiscard]] bool SetPointlightCollection(PointLightCollectionD3D11 *pointlights);

	[[nodiscard]] bool BeginSceneRender();
	[[nodiscard]] bool EndSceneRender(Time &time);

	[[nodiscard]] bool BeginUIRender() const;
	[[nodiscard]] bool RenderUI(Time &time);
	[[nodiscard]] bool EndUIRender() const;

	[[nodiscard]] bool EndFrame();
};
