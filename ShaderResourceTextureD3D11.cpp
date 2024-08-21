#include "ShaderResourceTextureD3D11.h"

#include "ErrMsg.h"


ShaderResourceTextureD3D11::~ShaderResourceTextureD3D11()
{
	if (_srv != nullptr)
		_srv->Release();
	if (_texture != nullptr)
		_texture->Release();
}


bool ShaderResourceTextureD3D11::Initialize(ID3D11Device* device, ID3D11DeviceContext* context, 
	const UINT width, const UINT height, const void* textureData, bool autoMipmaps)
{
	if (context == nullptr)
		autoMipmaps = false;

	D3D11_TEXTURE2D_DESC textureDesc = { };
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.ArraySize = 1;
	textureDesc.MipLevels = (autoMipmaps) ? 0u : 1u;
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

	if (autoMipmaps)
	{
		textureDesc.BindFlags |= D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
	}


	if (FAILED(device->CreateTexture2D(&textureDesc, (autoMipmaps) ? nullptr : &srData, &_texture)))
	{
		ErrMsg("Failed to create texture2D!");
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = { };
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = (autoMipmaps) ? unsigned(-1) : 1u;

	if (FAILED(device->CreateShaderResourceView(_texture, (autoMipmaps) ? &srvDesc : nullptr, &_srv)))
	{
		ErrMsg("Failed to create shader resource view!");
		return false;
	}

	if (autoMipmaps)
	{
		context->UpdateSubresource(_texture, 0, nullptr, srData.pSysMem, srData.SysMemPitch, 0);
		context->GenerateMips(_srv);
	}

	return true;
}

bool ShaderResourceTextureD3D11::Initialize(ID3D11Device *device, ID3D11DeviceContext* context, 
	const D3D11_TEXTURE2D_DESC &textureDesc, const D3D11_SUBRESOURCE_DATA *srData, bool autoMipmaps)
{
	if (context == nullptr)
		autoMipmaps = false;

	if (FAILED(device->CreateTexture2D(&textureDesc, (autoMipmaps) ? nullptr : srData, &_texture)))
	{
		ErrMsg("Failed to create texture2D!");
		return false;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = { };
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = (autoMipmaps) ? unsigned(-1) : 1u;

	if (FAILED(device->CreateShaderResourceView(_texture, (autoMipmaps) ? &srvDesc : nullptr, &_srv)))
	{
		ErrMsg("Failed to create shader resource view!");
		return false;
	}

	if (autoMipmaps)
	{
		context->UpdateSubresource(_texture, 0, nullptr, srData->pSysMem, srData->SysMemPitch, 0);
		context->GenerateMips(_srv);
	}

	return true;
}


ID3D11Texture2D *ShaderResourceTextureD3D11::GetTexture() const
{
	return _texture;
}

ID3D11ShaderResourceView *ShaderResourceTextureD3D11::GetSRV() const
{
	return _srv;
}
