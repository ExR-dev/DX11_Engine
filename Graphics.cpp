#include "Graphics.h"

#include "ErrMsg.h"
#include "Entity.h"
#include "D3D11Helper.h"


// private:

bool Graphics::FlushRenderQueue()
{
	// TODO: Sort render instances by shader, texture, etc. 


	// TODO: Draw sorted entities in batches to reduce state changes.

	size_t numInstances = _renderInstances.size();
	for (size_t i = 0; i < numInstances; i++)
	{
		const RenderInstance &instance = _renderInstances[i];

		_context->IASetInputLayout(_content->GetInputLayout(instance.inputLayoutID)->GetInputLayout());

		// Set the vertex and index buffers
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		_context->IASetVertexBuffers(0, 1, &instance.vertexBuffer, &stride, &offset);
		_context->IASetIndexBuffer(instance.indexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Set the shader
		_context->VSSetShader(instance.vertexShader, nullptr, 0);
		_context->PSSetShader(instance.pixelShader, nullptr, 0);

		// Set the texture
		_context->PSSetShaderResources(0, 1, &instance.texture);
		_context->PSSetSamplers(0, 1, &instance.sampler);

		// Set the constant buffer
		_context->VSSetConstantBuffers(0, 1, &instance.constantBuffer);

		// Draw the instance
		_context->DrawIndexed(instance.numIndices, 0, 0);
	}

	return true;
}

bool Graphics::ResetRenderState()
{
	_renderInstances.clear();
	_isRendering = false;

	return true;
}


// public:

Graphics::Graphics()
{
	_swapChain	= nullptr;
	_rtv		= nullptr;
	_dsTexture	= nullptr;
	_dsView		= nullptr;
	_viewport	= { };

	_context	= nullptr;
	_content	= nullptr;

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


bool Graphics::Setup(const UINT width, const UINT height, const HWND window, 
                     ID3D11Device *&device, ID3D11DeviceContext *&immediateContext, Content *content)
{
	if (_isSetup)
	{
		ErrMsg("Failed to set up graphics, graphics has already been set up!");
		return false;
	}

	if (!SetupD3D11(width, height, window, device, immediateContext, 
			_swapChain, _rtv, _dsTexture, _dsView, _viewport))
	{
		ErrMsg("Failed to setup d3d11!");
		return false;
	}

	_context = immediateContext;
	_content = content;

	_isSetup = true;
	return true;
}


ID3D11RenderTargetView *Graphics::GetRTV() const
{
	return _rtv;
}

ID3D11Texture2D *Graphics::GetDsTexture() const
{
	return _dsTexture;
}

ID3D11DepthStencilView *Graphics::GetDsView() const
{
	return _dsView;
}

D3D11_VIEWPORT &Graphics::GetViewport()
{
	return _viewport;
}


bool Graphics::BeginRender()
{
	if (!_isSetup)
	{
		ErrMsg("Failed to begin rendering, graphics has not been set up!");
		return false;
	}

	if (_isRendering)
	{
		ErrMsg("Failed to begin rendering, rendering has already begun!");
		return false;
	}
	_isRendering = true;

	constexpr float clearColour[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	_context->ClearRenderTargetView(_rtv, clearColour);
	_context->ClearDepthStencilView(_dsView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);

	_context->RSSetViewports(1, &_viewport);
	_context->OMSetRenderTargets(1, &_rtv, _dsView);

	_context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return true;
}

bool Graphics::QueueRender(const RenderInstance& instance)
{
	if (!_isRendering)
	{
		ErrMsg("Failed to queue render object, rendering has not begun!");
		return false;
	}

	_renderInstances.push_back(instance);

	return true;
}

bool Graphics::EndRender()
{
	if (!_isRendering)
	{
		ErrMsg("Failed to end rendering, rendering has not begun!");
		return false;
	}

	if (!FlushRenderQueue())
	{
		ErrMsg("Failed to flush render queue!");
		return false;
	}

	if (!ResetRenderState())
	{
		ErrMsg("Failed to reset render state!");
		return false;
	}

	return SUCCEEDED(_swapChain->Present(1, 0));
}
