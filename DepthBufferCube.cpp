#include "DepthBufferCube.h"

#include "ErrMsg.h"


DepthBufferCube::DepthBufferCube(
	ID3D11Device *device, const UINT width, const UINT height)
{
	if (!Initialize(device, width, height))
		ErrMsg("Failed to initialize depth buffer cube in constructor!");
}

DepthBufferCube::~DepthBufferCube()
{
	if (_srv != nullptr)
		_srv->Release();

	for (const auto &dsvArray : _dsvs)
		for (const auto &dsv : dsvArray)
			if (dsv != nullptr)
				dsv->Release();

	if (_texture != nullptr)
		_texture->Release();
}


bool DepthBufferCube::Initialize(
	ID3D11Device *device, const UINT width, const UINT height, const UINT arraySize)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = arraySize * 6;
	textureDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	if (FAILED(device->CreateTexture2D(&textureDesc, nullptr, &_texture)))
	{
		ErrMsg("Failed to create depth buffer texture cube!");
		return false;
	}


	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = { };
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
	dsvDesc.Texture2DArray.MipSlice = 0;
	dsvDesc.Texture2DArray.ArraySize = 1;

	_dsvs.reserve(arraySize);
	for (UINT i = 0; i < arraySize; i++)
	{
		std::array<ID3D11DepthStencilView *, 6> dsv;

		for (UINT j = 0; j < 6; ++j)
		{
			dsvDesc.Texture2DArray.FirstArraySlice = j;
			if (FAILED(device->CreateDepthStencilView(_texture, &dsvDesc, &dsv[j])))
			{
				ErrMsg(std::format("Failed to create depth stencil view #{}!", j));
				return false;
			}
		}
		_dsvs.push_back(dsv);
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = { };
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.MipLevels = 1;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.ArraySize = arraySize;

	if (FAILED(device->CreateShaderResourceView(_texture, &srvDesc, &_srv)))
	{
		ErrMsg("Failed to create depth buffer cube shader resource view!");
		return false;
	}

	return true;
}


ID3D11DepthStencilView *DepthBufferCube::GetDSV(const UINT arrayIndex, const UINT viewIndex) const
{
	return (_dsvs.at(arrayIndex)).at(viewIndex);
}

ID3D11ShaderResourceView *DepthBufferCube::GetSRV() const
{
	return _srv;
}
