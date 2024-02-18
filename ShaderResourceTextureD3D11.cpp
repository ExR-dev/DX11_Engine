#include "ShaderResourceTextureD3D11.h"

#include "ErrMsg.h"


ShaderResourceTextureD3D11::~ShaderResourceTextureD3D11()
{
	if (_srv != nullptr)
		_srv->Release();
	if (_texture != nullptr)
		_texture->Release();
}


bool ShaderResourceTextureD3D11::Initialize(ID3D11Device *device, const UINT width, const UINT height, const void *textureData)
{
	D3D11_TEXTURE2D_DESC textureDesc = { };
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.ArraySize = 1;
	textureDesc.MipLevels = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;

	D3D11_SUBRESOURCE_DATA srData = { };
	srData.pSysMem = textureData;
	srData.SysMemPitch = width * sizeof(unsigned char) * 4;
	srData.SysMemSlicePitch = 0;

	if (FAILED(device->CreateTexture2D(&textureDesc, &srData, &_texture)))
	{
		ErrMsg("Failed to create texture2D!");
		return false;
	}

	if (FAILED(device->CreateShaderResourceView(_texture, nullptr, &_srv)))
	{
		ErrMsg("Failed to create shader resource view!");
		return false;
	}

	return true;
}


ID3D11ShaderResourceView *ShaderResourceTextureD3D11::GetSRV() const
{
	return _srv;
}
