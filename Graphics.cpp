#include "Graphics.h"

#include "D3D11Helper.h"


Graphics::Graphics()
{
	_swapChain	= nullptr;
	_rtv		= nullptr;
	_dsTexture	= nullptr;
	_dsView		= nullptr;
	_viewport	= { };

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
		std::cerr << "Failed to setup d3d11!" << std::endl;
		return false;
	}

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


bool Graphics::BeginRender(ID3D11DeviceContext *&immediateContext, DebugData &debugData)
{
	if (!_isSetup)
		return false;
	if (_isRendering)
		return false;

	constexpr float clearColour[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	immediateContext->ClearRenderTargetView(_rtv, clearColour);
	immediateContext->ClearDepthStencilView(_dsView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

	immediateContext->RSSetViewports(1, &_viewport);
	immediateContext->OMSetRenderTargets(1, &_rtv, _dsView);

	_isRendering = true;
	return true;
}

bool Graphics::Render(const Entity &entity, UINT &vertexCount)
{
	if (!_isRendering)
		return false;

	// TODO

	return true;
}

bool Graphics::EndRender(ID3D11DeviceContext *&immediateContext, UINT &vertexCount)
{
	if (!_isRendering)
		return false;

	immediateContext->Draw(vertexCount, 0);
	_swapChain->Present(0, 0);

	_isRendering = false;
	return true;
}
