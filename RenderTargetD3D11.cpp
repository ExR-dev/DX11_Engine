#include "RenderTargetD3D11.h"

#include "ErrMsg.h"


RenderTargetD3D11::~RenderTargetD3D11()
{
	if (_srv != nullptr)
		_srv->Release();

	if (_rtv != nullptr)
		_rtv->Release();

	if (_texture != nullptr)
		_texture->Release();
}

bool RenderTargetD3D11::Initialize(ID3D11Device *device, const UINT width, const UINT height, const DXGI_FORMAT format, const bool hasSRV)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = format;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	if (hasSRV)
		textureDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

	if (FAILED(device->CreateTexture2D(&textureDesc, nullptr, &_texture)))
	{
		ErrMsg("Failed to create render target texture!");
		return false;
	}

	if (hasSRV)
	{
		/*D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;*/

		if (FAILED(device->CreateShaderResourceView(_texture, /*&srvDesc*/ nullptr, &_srv)))
		{
			ErrMsg("Failed to create shader resource view!");
			return false;
		}
	}

	if (FAILED(device->CreateRenderTargetView(_texture, nullptr, &_rtv)))
	{
		ErrMsg("Failed to create render target view!");
		return false;
	}

	return true;
}


ID3D11RenderTargetView *RenderTargetD3D11::GetRTV() const
{
	return _rtv;
}

ID3D11ShaderResourceView *RenderTargetD3D11::GetSRV() const
{
	return _srv;
}
