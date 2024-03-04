#pragma once

#include <d3d11.h>
#include <map>
#include <array>

#include "CameraD3D11.h"
#include "Content.h"
//#include "PointLightCollectionD3D11.h"
#include "RenderTargetD3D11.h"
#include "SpotLightCollectionD3D11.h"
#include "Time.h"


constexpr UINT G_BUFFER_COUNT = 3;


struct ResourceGroup
{
	UINT
		meshID = CONTENT_LOAD_ERROR,
		vsID = CONTENT_LOAD_ERROR,
		psID = CONTENT_LOAD_ERROR,
		texID = CONTENT_LOAD_ERROR,
		samplerID = CONTENT_LOAD_ERROR,
		inputLayoutID = CONTENT_LOAD_ERROR;

    bool operator<(const ResourceGroup& other) const
    {
        if (meshID != other.meshID)
            return meshID < other.meshID;

        if (texID != other.texID)
            return texID < other.texID;

        if (vsID != other.vsID)
            return vsID < other.vsID;

        if (psID != other.psID)
            return psID < other.psID;

        if (samplerID != other.samplerID)
            return samplerID < other.samplerID;

        return inputLayoutID < other.inputLayoutID;
    }
};

struct RenderInstance
{
	void *subject;
	size_t subjectSize;
};


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
	ID3D11DepthStencilView *_dsView = nullptr;
	ID3D11UnorderedAccessView *_uav = nullptr;
	D3D11_VIEWPORT _viewport		= { };

	std::array<RenderTargetD3D11, G_BUFFER_COUNT> _gBuffers;

	ConstantBufferD3D11 _globalLightBuffer;
	CameraD3D11 *_currCamera = nullptr;
	//PointLightCollectionD3D11 *_currPointLightCollection = nullptr;
	SpotLightCollectionD3D11 *_currSpotLightCollection = nullptr;
	std::multimap<ResourceGroup, RenderInstance> _renderInstances; // Let batching be handled by multimap

	UINT
		_currMeshID			= CONTENT_LOAD_ERROR,
		_currVsID			= CONTENT_LOAD_ERROR,
		_currPsID			= CONTENT_LOAD_ERROR,
		_currTexID			= CONTENT_LOAD_ERROR,
		_currSamplerID		= CONTENT_LOAD_ERROR,
		_currInputLayoutID	= CONTENT_LOAD_ERROR;


	[[nodiscard]] bool RenderShadowCasters();
	[[nodiscard]] bool RenderGeometry();
	[[nodiscard]] bool RenderLighting();

	[[nodiscard]] bool ResetRenderState();

public:
	~Graphics();

	[[nodiscard]] bool Setup(UINT width, UINT height, HWND window,
		ID3D11Device *&device, ID3D11DeviceContext *&immediateContext, Content *content);

	[[nodiscard]] ID3D11RenderTargetView *GetRTV() const;
	[[nodiscard]] ID3D11Texture2D *GetDsTexture() const;
	[[nodiscard]] ID3D11DepthStencilView *GetDsView() const;
	[[nodiscard]] D3D11_VIEWPORT &GetViewport();

	[[nodiscard]] bool SetCamera(CameraD3D11 *camera);
	[[nodiscard]] bool SetSpotlightCollection(SpotLightCollectionD3D11 *spotlights);
	//[[nodiscard]] bool SetPointlightCollection(PointLightCollectionD3D11 *pointlights);

	[[nodiscard]] bool BeginSceneRender();
	[[nodiscard]] bool QueueRenderInstance(const ResourceGroup &resources, const RenderInstance &instance);
	[[nodiscard]] bool EndSceneRender(const Time &time);

	[[nodiscard]] bool BeginUIRender() const;
	[[nodiscard]] bool RenderUI(const Time &time) const;
	[[nodiscard]] bool EndUIRender() const;

	[[nodiscard]] bool EndFrame();
};
