#include "DepthBufferD3D11.h"

#include "ErrMsg.h"


DepthBufferD3D11::DepthBufferD3D11(
	ID3D11Device *device, const UINT width, const UINT height, const bool hasSRV)
{
	if (!Initialize(device, width, height, hasSRV))
		ErrMsg("Failed to initialize depth buffer in constructor!");
}

DepthBufferD3D11::~DepthBufferD3D11()
{
	if (_srv != nullptr)
		_srv->Release();

	for (const auto &dsv : _depthStencilViews)
	{
		if (dsv != nullptr)
			dsv->Release();
	}

	if (_texture != nullptr)
		_texture->Release();
}


bool DepthBufferD3D11::Initialize(
	ID3D11Device *device, const UINT width, const UINT height, 
	const bool hasSRV, const UINT arraySize)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = arraySize;
	textureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	if (hasSRV)
	{
		textureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	}

	if (FAILED(device->CreateTexture2D(&textureDesc, nullptr, &_texture)))
	{
		ErrMsg("Failed to create depth buffer texture!");
		return false;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = { };
	const D3D11_DEPTH_STENCIL_VIEW_DESC *dsvDescPtr = nullptr;

	if (hasSRV)
	{
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
		dsvDesc.Texture2DArray.MipSlice = 0;
		dsvDesc.Texture2DArray.ArraySize = 1;

		dsvDescPtr = &dsvDesc;
	}

	_depthStencilViews.reserve(arraySize);
	for (UINT i = 0; i < arraySize; i++)
	{
		if (hasSRV)
			dsvDesc.Texture2DArray.FirstArraySlice = i;

		ID3D11DepthStencilView *dsView;
		if (FAILED(device->CreateDepthStencilView(_texture, dsvDescPtr, &dsView)))
		{
			ErrMsg(std::format("Failed to create depth stencil view #{}!", i));
			return false;
		}
		_depthStencilViews.push_back(dsView);
	}

	if (hasSRV)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = { };
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Texture2DArray.MostDetailedMip = 0;
		srvDesc.Texture2DArray.MipLevels = 1;
		srvDesc.Texture2DArray.FirstArraySlice = 0;
		srvDesc.Texture2DArray.ArraySize = arraySize;

		if (FAILED(device->CreateShaderResourceView(_texture, &srvDesc, &_srv)))
		{
			ErrMsg("Failed to create depth buffer shader resource view!");
			return false;
		}
	}

	return true;
}


ID3D11DepthStencilView *DepthBufferD3D11::GetDSV(const UINT arrayIndex) const
{
	return _depthStencilViews.at(arrayIndex);
}

ID3D11ShaderResourceView *DepthBufferD3D11::GetSRV() const
{
	return _srv;
}
