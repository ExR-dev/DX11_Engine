#include "Graphics.h"

#include <iostream>

#include "D3D11Helper.h"
#include "ErrMsg.h"


Graphics::Graphics()
{
	_swapChain	= nullptr;
	_rtv		= nullptr;
	_dsTexture	= nullptr;
	_dsView		= nullptr;
	_viewport	= { };

	_context	= nullptr;

	_isSetup		= false;
	_isRendering	= false;
}

Graphics::~Graphics()
{
	if (_dsView != nullptr)
		_dsView->Release();

	if (_dsTexture != nullptr)
		_dsTexture->Release();

	if (_rtv != nullptr)
		_rtv->Release();

	if (_swapChain != nullptr)
		_swapChain->Release();
}

bool Graphics::Setup(
	UINT width, UINT height, HWND window, 
	ID3D11Device *&device, ID3D11DeviceContext *&immediateContext)
{
	if (_isSetup)
		return false;

	if (!SetupD3D11(width, height, window, device, immediateContext, 
			_swapChain, _rtv, _dsTexture, _dsView, _viewport))
	{
		ErrMsg("Failed to setup d3d11!");
		return false;
	}

	_context = immediateContext;
	_isSetup = true;
	return true;
}


ID3D11RenderTargetView *Graphics::GetRTV()
{
	return _rtv;
}

ID3D11Texture2D *Graphics::GetDsTexture()
{
	return _dsTexture;
}

ID3D11DepthStencilView *Graphics::GetDsView()
{
	return _dsView;
}

D3D11_VIEWPORT &Graphics::GetViewport()
{
	return _viewport;
}


bool Graphics::BeginRender(const Content &content)
{
	if (!_isSetup)
		return false;
	if (_isRendering)
		return false;

	constexpr float clearColour[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	_context->ClearRenderTargetView(_rtv, clearColour);
	_context->ClearDepthStencilView(_dsView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

	_context->RSSetViewports(1, &_viewport);
	_context->OMSetRenderTargets(1, &_rtv, _dsView);

	_context->IASetInputLayout(content.GetInputLayout(0)->GetInputLayout());
	_context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	_currInputLayoutID = 0;

	_isRendering = true;
	return true;
}

bool Graphics::EndRender()
{
	if (!_isRendering)
		return false;

	_swapChain->Present(1, 0);

	_isRendering = false;
	return true;
}


bool Graphics::Render(Entity &entity, const Content &content)
{
	if (!_isRendering)
		return false;

	// TODO
	_currInputLayoutID = CONTENT_LOAD_ERROR;
	_currMeshID = CONTENT_LOAD_ERROR;
	_currVsID = CONTENT_LOAD_ERROR;
	_currPsID = CONTENT_LOAD_ERROR;
	_currTexID = CONTENT_LOAD_ERROR;

	return true;
}
