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


bool Graphics::BeginRender()
{
	if (!_isSetup)
		return false;
	if (_isRendering)
		return false;

	// TODO

	_isRendering = true;
	return true;
}

bool Graphics::Render(const Entity &entity)
{
	if (!_isRendering)
		return false;

	// TODO

	return true;
}

bool Graphics::EndRender()
{
	if (!_isRendering)
		return false;

	_swapChain->Present(0, 0);

	_isRendering = false;
	return true;
}