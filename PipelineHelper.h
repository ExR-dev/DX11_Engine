#pragma once

#include <array>
#include <d3d11.h>
#include <DirectXMath.h>
#include <string>

using namespace DirectX;


struct SimpleVertex 
{
	float pos[3];
	float norm[3];
	float uv[2];

	SimpleVertex(const std::array<float, 3> &position, const std::array<float, 3> &normal, const std::array<float, 2> &texCoords)
	{
		for (int i = 0; i < 3; ++i)
		{
			pos[i] = position[i];
			norm[i] = normal[i];
		}

		uv[0] = texCoords[0];
		uv[1] = texCoords[1];
	}
};

struct WorldMatrixBufferData
{
	XMFLOAT4X4 worldMatrix;
	
	WorldMatrixBufferData()
	{
		XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
	}
};

struct ViewProjMatrixBufferData
{
	XMFLOAT4X4 viewProjMatrix; 
	
	ViewProjMatrixBufferData()
	{
		XMStoreFloat4x4(&viewProjMatrix, XMMatrixIdentity());
	}
};

struct LightingBufferData
{
	float camPos[4];
	float lightPos[4];

	float ambCol[4];
	float diffCol[4];
	float specCol[4];

	LightingBufferData(
		const std::array<float, 4> &cameraPosition, const std::array<float, 4> &lightPosition, 
		const std::array<float, 4> &ambientColour, const std::array<float, 4> &diffuseColour, const std::array<float, 4> &specularColour)
	{
		for (int i = 0; i < 4; ++i)
		{
			camPos[i] = cameraPosition[i];
			lightPos[i] = lightPosition[i];

			ambCol[i] = ambientColour[i];
			diffCol[i] = diffuseColour[i];
			specCol[i] = specularColour[i];
		}
	}
};


bool LoadShaders(
	ID3D11Device *device,
	ID3D11VertexShader *&vShader,
	ID3D11PixelShader *&pShader,
	std::string &vShaderByteCode
);

bool LoadTexture2D(
	ID3D11Device *device,
	ID3D11Texture2D *&texture2D,
	ID3D11ShaderResourceView *&resourceView,
	ID3D11SamplerState *&samplerState
);


bool CreateInputLayout(
	ID3D11Device *device,
	ID3D11InputLayout *&inputLayout,
	const std::string &vShaderByteCode
);
