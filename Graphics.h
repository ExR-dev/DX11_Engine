#pragma once

#include <d3d11.h>
#include <array>
#include "Content.h"
#include "Time.h"
#include "Cubemap.h"
#include "RenderTargetD3D11.h"
#include "CameraD3D11.h"
#include "SpotLightCollectionD3D11.h"
#include "DirLightCollectionD3D11.h"
#include "PointLightCollectionD3D11.h"


// Handles rendering of the scene and the GUI.
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
	bool _updateCubemap = false;

	ConstantBufferD3D11 _globalLightBuffer;
	SpotLightCollectionD3D11 *_currSpotLightCollection = nullptr;
	DirLightCollectionD3D11 *_currDirLightCollection = nullptr;
	PointLightCollectionD3D11 *_currPointLightCollection = nullptr;

	UINT
		_currMeshID			= CONTENT_NULL,
		_currVsID			= CONTENT_NULL,
		_currPsID			= CONTENT_NULL,
		_currTexID			= CONTENT_NULL,
		_currNormalID		= CONTENT_NULL,
		_currSpecularID		= CONTENT_NULL,
		_currReflectiveID	= CONTENT_NULL,
		_currAmbientID		= CONTENT_NULL,
		_currHeightID		= CONTENT_NULL,
		_currSamplerID		= CONTENT_NULL,
		_currInputLayoutID	= CONTENT_NULL;

	DirectX::XMFLOAT4A _ambientColor = { 0.0f, 0.0f, 0.0f, 0.0f };
	bool _renderTransparency = true;

	// Renders all queued entities to the specified target.
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
	[[nodiscard]] bool RenderDirlights();
	[[nodiscard]] bool RenderPointlights();

	// Renders all queued opaque entities to the depth buffers of all shadow-casting lights.
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
	[[nodiscard]] bool SetDirlightCollection(DirLightCollectionD3D11 *dirlights);
	[[nodiscard]] bool SetPointlightCollection(PointLightCollectionD3D11 *pointlights);

	// Begins scene rendering, enabling entities to be queued for rendering.
	[[nodiscard]] bool BeginSceneRender();

	// Renders all queued entities to the window.
	[[nodiscard]] bool EndSceneRender(Time &time);

	[[nodiscard]] bool BeginUIRender() const;
	[[nodiscard]] bool RenderUI(Time &time);
	[[nodiscard]] bool EndUIRender() const;

	// Resets variables and clears all render queues.
	[[nodiscard]] bool EndFrame();
};
