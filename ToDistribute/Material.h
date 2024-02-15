#pragma once

#include <string>
#include <d3d11_4.h>


struct MaterialData 
{

};


class Material
{
private:
	ID3D11Texture2D* _ambientTexture = nullptr;
	ID3D11Texture2D* _diffuseTexture = nullptr;
	ID3D11Texture2D* _specularTexture = nullptr;

public:
	Material();
	~Material();
	Material(const Material& other) = default;
	Material& operator=(const Material& other) = default;
	Material(Material&& other) = default;
	Material& operator=(Material&& other) = default;

	bool Initialize(ID3D11Device *device, ID3D11Resource *ambientData, ID3D11Resource*diffuseData, ID3D11Resource *specularData);

	ID3D11Texture2D* GetAmbientTex() const;
	ID3D11Texture2D* GetDiffuseTex() const;
	ID3D11Texture2D* GetSpecularTex() const;
};