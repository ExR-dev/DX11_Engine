#pragma once

#include <d3d11_4.h>


enum class TextureType
{
	DEFAULT = 0,
	NORMAL = 1,
	SPECULAR = 2,
	REFLECTIVE = 3,
};


class ShaderResourceTextureD3D11
{
private:
	ID3D11Texture2D *_texture = nullptr;
	ID3D11ShaderResourceView *_srv = nullptr;

public:
	ShaderResourceTextureD3D11() = default;
	~ShaderResourceTextureD3D11();

	ShaderResourceTextureD3D11(const ShaderResourceTextureD3D11 &other) = delete;
	ShaderResourceTextureD3D11 &operator=(const ShaderResourceTextureD3D11 &other) = delete;
	ShaderResourceTextureD3D11(ShaderResourceTextureD3D11 &&other) = delete;
	ShaderResourceTextureD3D11 &operator=(ShaderResourceTextureD3D11 &&other) = delete;

	[[nodiscard]] bool Initialize(ID3D11Device *device, UINT width, UINT height, const void *textureData);
	[[nodiscard]] bool Initialize(ID3D11Device *device, const D3D11_TEXTURE2D_DESC &textureDesc, const D3D11_SUBRESOURCE_DATA *srData);

	[[nodiscard]] ID3D11Texture2D *GetTexture() const;
	[[nodiscard]] ID3D11ShaderResourceView *GetSRV() const;
};