#pragma once

#include <d3d11_4.h>


class Material
{
private:
	// ID3D11Texture2D *_ambientTexture = nullptr;
	// ID3D11Texture2D *_diffuseTexture = nullptr;
	// ID3D11Texture2D *_specularTexture = nullptr;

public:
	Material();
	~Material();
	Material(const Material &other) = delete;
	Material &operator=(const Material &other) = delete;
	Material(Material&& other) = delete;
	Material &operator=(Material &&other) = delete;

	bool Initialize(ID3D11Device *device);

	ID3D11Texture2D *GetAmbientTex() const;
	ID3D11Texture2D *GetDiffuseTex() const;
	ID3D11Texture2D *GetSpecularTex() const;
};