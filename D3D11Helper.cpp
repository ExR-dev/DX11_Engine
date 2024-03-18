#include "D3D11Helper.h"

#include "Graphics.h"
#include "ErrMsg.h"


bool CreateInterfaces(
	const UINT width, const UINT height, const HWND window,
	ID3D11Device *&device, 
	ID3D11DeviceContext *&immediateContext, 
	IDXGISwapChain *&swapChain)
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};

	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS;
	swapChainDesc.OutputWindow = window;
	swapChainDesc.Windowed = true;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.Flags = 0;

#ifdef _DEBUG
	UINT flags = D3D11_CREATE_DEVICE_DEBUG;
#else
	UINT flags = 0;
#endif

	constexpr D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };

	return SUCCEEDED(D3D11CreateDeviceAndSwapChain(
		nullptr, D3D_DRIVER_TYPE_HARDWARE,
		nullptr, flags, featureLevels,
		1, D3D11_SDK_VERSION, &swapChainDesc,
		&swapChain, &device, nullptr, &immediateContext
	));
}

bool CreateRenderTargetView(
	ID3D11Device *device, 
	IDXGISwapChain *swapChain, 
	ID3D11RenderTargetView *&rtv,
	ID3D11UnorderedAccessView *&uav)
{
	// get the address of the back buffer
	ID3D11Texture2D *backBuffer = nullptr;
	if (FAILED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void **>(&backBuffer))))
	{
		ErrMsg("Failed to get back buffer!");
		return false;
	}

	// use the back buffer address to create the render target
	// null as description to base it on the backbuffers values
	if (FAILED(device->CreateRenderTargetView(backBuffer, nullptr, &rtv)))
	{
		ErrMsg("Failed to create render target view!");
		return false;
	}

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = { };
	uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;

	if (FAILED(device->CreateUnorderedAccessView(backBuffer, &uavDesc, &uav)))
	{
		ErrMsg("Failed to create unordered access view!");
		return false;
	}

	backBuffer->Release();
	return true;
}

bool CreateDepthStencil(
	ID3D11Device *device, const UINT width, const UINT height, 
	ID3D11Texture2D *&dsTexture, 
	ID3D11DepthStencilView *&dsView)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	if (FAILED(device->CreateTexture2D(&textureDesc, nullptr, &dsTexture)))
	{
		ErrMsg("Failed to create depth stencil texture!");
		return false;
	}

	return SUCCEEDED(device->CreateDepthStencilView(dsTexture, nullptr, &dsView));
}

bool CreateBlendState(ID3D11Device *device, ID3D11BlendState *&blendState)
{
	D3D11_BLEND_DESC blendDesc = { };
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;

	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MAX;

	/*blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;

	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;*/

	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	if (FAILED(device->CreateBlendState(&blendDesc, &blendState)))
	{
		ErrMsg("Failed to create blend state!");
		return false;
	}

	return true;
}

bool CreateDepthStencilState(ID3D11Device *device, ID3D11DepthStencilState *&depthStencilState)
{
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = { };
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = false;
	depthStencilDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	depthStencilDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;

	if (FAILED(device->CreateDepthStencilState(&depthStencilDesc, &depthStencilState)))
	{
		ErrMsg("Failed to create depth stencil state!");
		return false;
	}

	return true;
}

void SetViewport(D3D11_VIEWPORT &viewport, const UINT width, const UINT height)
{
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;
}


bool SetupD3D11(
	const UINT width, const UINT height, const HWND window, 
	ID3D11Device *&device,
	ID3D11DeviceContext *&immediateContext, 
	IDXGISwapChain *&swapChain, 
	ID3D11RenderTargetView *&rtv,
	ID3D11Texture2D *&dsTexture, 
	ID3D11DepthStencilView *&dsView, 
	ID3D11UnorderedAccessView *&uav, 
	ID3D11BlendState *&blendState,
	ID3D11DepthStencilState *&depthStencilState,
	D3D11_VIEWPORT &viewport)
{
	if (!CreateInterfaces(width, height, window, device, immediateContext, swapChain))
	{
		ErrMsg("Error creating interfaces!");
		return false;
	}

	if (!CreateRenderTargetView(device, swapChain, rtv, uav))
	{
		ErrMsg("Error creating rtv!");
		return false;
	}

	if (!CreateDepthStencil(device, width, height, dsTexture, dsView))
	{
		ErrMsg("Error creating depth stencil view!");
		return false;
	}

	if (!CreateBlendState(device, blendState))
	{
		ErrMsg("Error creating blend state!");
		return false;
	}

	if (!CreateDepthStencilState(device, depthStencilState))
	{
		ErrMsg("Error creating depth stencil state!");
		return false;
	}

	SetViewport(viewport, width, height);

	return true;
}
