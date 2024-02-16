#include "PipelineHelper.h"

#include <fstream>
#include <string>
#include <iostream>
#include <DirectXMath.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "ErrMsg.h"

using namespace DirectX;


bool LoadShaders(
	ID3D11Device *device, 
	ID3D11VertexShader *&vShader, 
	ID3D11PixelShader *&pShader, 
	std::string &vShaderByteCode)
{
	std::string shaderData;
	std::ifstream reader;

	std::string shaderPath = "";

#ifdef _WIN64
	shaderPath = "x64/";
#endif

#ifdef _DEBUG
	shaderPath += "Debug/";
#else
	shaderPath += "Release/";
#endif 

	reader.open(shaderPath + "VertexShader.cso", std::ios::binary | std::ios::ate);
	if (!reader.is_open())
	{
		ErrMsg("Could not open VS file!");
		return false;
	}

	reader.seekg(0, std::ios::end);
	shaderData.reserve(static_cast<unsigned int>(reader.tellg()));
	reader.seekg(0, std::ios::beg);

	shaderData.assign(
		std::istreambuf_iterator<char>(reader),
		std::istreambuf_iterator<char>()
	);

	if (FAILED(device->CreateVertexShader(
			shaderData.c_str(), 
			shaderData.length(), 
			nullptr, 
			&vShader)))
	{
		ErrMsg("Failed to create vertex shader!");
		return false;
	}

	vShaderByteCode = shaderData;
	shaderData.clear();
	reader.close();
	reader.open(shaderPath + "PixelShader.cso", std::ios::binary | std::ios::ate);
	if (!reader.is_open())
	{
		ErrMsg("Could not open PS file!");
		return false;
	}

	reader.seekg(0, std::ios::end);
	shaderData.reserve(static_cast<unsigned int>(reader.tellg()));
	reader.seekg(0, std::ios::beg);

	shaderData.assign(
		std::istreambuf_iterator<char>(reader),
		std::istreambuf_iterator<char>()
	);

	if (FAILED(device->CreatePixelShader(
			shaderData.c_str(), 
			shaderData.length(), 
			nullptr, 
			&pShader)))
	{
		ErrMsg("Failed to create pixel shader!");
		return false;
	}

	return true;
}


bool CreateInputLayout(
	ID3D11Device *device, 
	ID3D11InputLayout *&inputLayout, 
	const std::string &vShaderByteCode)
{
	constexpr D3D11_INPUT_ELEMENT_DESC inputDesc[4] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	HRESULT hr = device->CreateInputLayout(
		inputDesc, 3, 
		vShaderByteCode.c_str(), 
		vShaderByteCode.length(), 
		&inputLayout
	);

	return !(FAILED(hr));
}

bool CreateVertexBuffer(
	ID3D11Device *device, 
	ID3D11Buffer *&vertexBuffer,
	SimpleVertex *vertices, UINT vertexCount)
{
	D3D11_BUFFER_DESC vertexBufferDesc = { };
	vertexBufferDesc.ByteWidth = sizeof(*vertices) * vertexCount;
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA srData = { };
	srData.pSysMem = vertices;
	srData.SysMemPitch = 0;
	srData.SysMemSlicePitch = 0;

	HRESULT hr = device->CreateBuffer(&vertexBufferDesc, &srData, &vertexBuffer);
	return !(FAILED(hr));
}

bool CreateMatrixBuffer(
	ID3D11Device *device, 
	ID3D11Buffer *&matrixBuffer)
{
	MatrixBufferData matrixBufferData = { };

	D3D11_BUFFER_DESC matrixBufferDesc = { };
	matrixBufferDesc.ByteWidth = sizeof(matrixBufferData);
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA srData = { };
	srData.pSysMem = &matrixBufferData;
	srData.SysMemPitch = 0;
	srData.SysMemSlicePitch = 0;

	HRESULT hr = device->CreateBuffer(&matrixBufferDesc, &srData, &matrixBuffer);
	return !(FAILED(hr));
}

bool UpdateMatrixBuffer(
	ID3D11DeviceContext *immediateContext,
	ID3D11Buffer *&matrixBuffer,
	MatrixBufferData &matrixBufferData)
{
	D3D11_MAPPED_SUBRESOURCE resource;
	HRESULT hr = immediateContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &resource);
	if (FAILED(hr))
		return false;

	memcpy(resource.pData, &matrixBufferData, sizeof(MatrixBufferData));
	immediateContext->Unmap(matrixBuffer, 0);
	return true;
}

bool CreateLightingBuffer(
	ID3D11Device *device,
	ID3D11Buffer *&lightingBuffer)
{
	LightingBufferData lightingBufferData = {
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
		{0,0,0,0},
	};

	D3D11_BUFFER_DESC lightingBufferDesc = { };
	lightingBufferDesc.ByteWidth = sizeof(lightingBufferData);
	lightingBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightingBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightingBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightingBufferDesc.MiscFlags = 0;
	lightingBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA srData = { };
	srData.pSysMem = &lightingBufferData;
	srData.SysMemPitch = 0;
	srData.SysMemSlicePitch = 0;

	HRESULT hr = device->CreateBuffer(&lightingBufferDesc, &srData, &lightingBuffer);
	return !(FAILED(hr));
}

bool UpdateLightingBuffer(
	ID3D11DeviceContext *immediateContext,
	ID3D11Buffer *&lightingBuffer,
	LightingBufferData &lightingBufferData)
{
	D3D11_MAPPED_SUBRESOURCE resource = { };
	HRESULT hr = immediateContext->Map(lightingBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &resource);
	if (FAILED(hr))
		return false;

	memcpy(resource.pData, &lightingBufferData, sizeof(LightingBufferData));
	immediateContext->Unmap(lightingBuffer, 0);
	return true;
}

bool LoadTexture2D(
	ID3D11Device *device,
	ID3D11Texture2D *&texture2D,
	ID3D11ShaderResourceView *&resourceView,
	ID3D11SamplerState *&samplerState)
{
	int x,y,n;
	unsigned char *data = stbi_load("texture.png", &x, &y, &n, 4);

	D3D11_TEXTURE2D_DESC textureDesc = { };
	textureDesc.Width = x;
	textureDesc.Height = y;
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
	srData.pSysMem = data;
	srData.SysMemPitch = x * sizeof(unsigned char) * 4;
	srData.SysMemSlicePitch = 0;

	if (FAILED(device->CreateTexture2D(&textureDesc, &srData, &texture2D)))
	{
		ErrMsg("Error creating texture2D!");
		stbi_image_free(data);
		return false;
	}

	if (FAILED(device->CreateShaderResourceView(texture2D, nullptr, &resourceView)))
	{
		ErrMsg("Error creating shader resource view!");
		stbi_image_free(data);
		return false;
	}

	D3D11_SAMPLER_DESC samplerDesc = { };
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = 0;
	samplerDesc.MaxAnisotropy = 0;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	if (FAILED(device->CreateSamplerState(&samplerDesc, &samplerState)))
	{
		ErrMsg("Error creating sampler state!");
		stbi_image_free(data);
		return false;
	}
	stbi_image_free(data);
	return true;
}


bool SetupPipeline(
	ID3D11Device *device, 
	//ID3D11Buffer *&vertexBuffer,
	ID3D11Buffer *&matrixBuffer,
	ID3D11Buffer *&lightingBuffer,
	ID3D11Texture2D *&texture2D,
	ID3D11ShaderResourceView *&resourceView,
	ID3D11SamplerState *&samplerState,
	ID3D11VertexShader *&vShader,
	ID3D11PixelShader *&pShader, 
	ID3D11InputLayout *&inputLayout,
	SimpleVertex *vertices, UINT vertexCount)
{
	std::string vShaderByteCode;
	if (!LoadShaders(device, vShader, pShader, vShaderByteCode))
	{
		ErrMsg("Error loading shaders!");
		return false;
	}

	if (!CreateInputLayout(device, inputLayout, vShaderByteCode))
	{
		ErrMsg("Error creating input layout!");
		return false;
	}

	/*if (!CreateVertexBuffer(device, vertexBuffer, vertices, vertexCount))
	{
		ErrMsg("Error creating vertex buffer!");
		return false;
	}*/

	if (!CreateMatrixBuffer(device, matrixBuffer))
	{
		ErrMsg("Error creating matrix buffer!");
		return false;
	}

	if (!CreateLightingBuffer(device, lightingBuffer))
	{
		ErrMsg("Error creating lighting buffer!");
		return false;
	}

	if (!LoadTexture2D(device, texture2D, resourceView, samplerState))
	{
		ErrMsg("Error loading 2D texture!");
		return false;
	}

	return true;
}