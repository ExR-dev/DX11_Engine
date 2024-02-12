#include "ShaderD3D11.h"

#include <d3dcompiler.h>
#include <fstream>
#include <string>
#include <iostream>


ShaderD3D11::~ShaderD3D11()
{
	switch (_type)
	{
		case ShaderType::VERTEX_SHADER:
			_shader.vertex->Release();
			break;

		case ShaderType::HULL_SHADER:
			_shader.hull->Release();
			break;

		case ShaderType::DOMAIN_SHADER:
			_shader.domain->Release();
			break;

		case ShaderType::GEOMETRY_SHADER:
			_shader.geometry->Release();
			break;

		case ShaderType::PIXEL_SHADER:
			_shader.pixel->Release();
			break;

		case ShaderType::COMPUTE_SHADER:
			_shader.compute->Release();
			break;
	}

	_shaderBlob->Release();
}


void ShaderD3D11::Initialize(ID3D11Device *device, const ShaderType shaderType, const void *dataPtr, const size_t dataSize)
{
	D3DCreateBlob(dataSize, &_shaderBlob);
	std::memcpy(_shaderBlob->GetBufferPointer(), dataPtr, dataSize);
	const void *shaderData = _shaderBlob->GetBufferPointer();

	_type = shaderType;
	switch (_type)
	{
		case ShaderType::VERTEX_SHADER:
			if (FAILED(device->CreateVertexShader(
					shaderData, dataSize,
					nullptr, &_shader.vertex)))
			{
				std::cerr << "Failed to create vertex shader!" << std::endl;
			}
			break;

		case ShaderType::HULL_SHADER:
			if (FAILED(device->CreateHullShader(
					shaderData, dataSize,
					nullptr, &_shader.hull)))
			{
				std::cerr << "Failed to create hull shader!" << std::endl;
			}
			break;

		case ShaderType::DOMAIN_SHADER:
			if (FAILED(device->CreateDomainShader(
					shaderData, dataSize,
					nullptr, &_shader.domain)))
			{
				std::cerr << "Failed to create domain shader!" << std::endl;
			}
			break;

		case ShaderType::GEOMETRY_SHADER:
			if (FAILED(device->CreateGeometryShader(
					shaderData, dataSize,
					nullptr, &_shader.geometry)))
			{
				std::cerr << "Failed to create geometry shader!" << std::endl;
			}
			break;

		case ShaderType::PIXEL_SHADER:
			if (FAILED(device->CreatePixelShader(
					shaderData, dataSize,
					nullptr, &_shader.pixel)))
			{
				std::cerr << "Failed to create pixel shader!" << std::endl;
			}
			break;

		case ShaderType::COMPUTE_SHADER:
			if (FAILED(device->CreateComputeShader(
					shaderData, dataSize,
					nullptr, &_shader.compute)))
			{
				std::cerr << "Failed to create compute shader!" << std::endl;
			}
			break;
	}
}

void ShaderD3D11::Initialize(ID3D11Device *device, const ShaderType shaderType, const char *csoPath)
{
	std::string shaderFileData;
	std::ifstream reader;

	reader.open(csoPath, std::ios::binary | std::ios::ate);
	if (!reader.is_open())
		std::cerr << "Could not open shader file!" << std::endl;

	reader.seekg(0, std::ios::end);
	shaderFileData.reserve(static_cast<unsigned int>(reader.tellg()));
	reader.seekg(0, std::ios::beg);

	shaderFileData.assign(
		std::istreambuf_iterator<char>(reader),
		std::istreambuf_iterator<char>()
	);

	Initialize(device, shaderType, shaderFileData.c_str(), shaderFileData.length());

	shaderFileData.clear();
	reader.close();
}


void ShaderD3D11::BindShader(ID3D11DeviceContext *context) const
{
	switch (_type)
	{
		case ShaderType::VERTEX_SHADER:
			context->VSSetShader(_shader.vertex, nullptr, 0);
			break;

		case ShaderType::HULL_SHADER:
			context->HSSetShader(_shader.hull, nullptr, 0);
			break;

		case ShaderType::DOMAIN_SHADER:
			context->DSSetShader(_shader.domain, nullptr, 0);
			break;

		case ShaderType::GEOMETRY_SHADER:
			context->GSSetShader(_shader.geometry, nullptr, 0);
			break;

		case ShaderType::PIXEL_SHADER:
			context->PSSetShader(_shader.pixel, nullptr, 0);
			break;

		case ShaderType::COMPUTE_SHADER:
			context->CSSetShader(_shader.compute, nullptr, 0);
			break;
	}
}


const void *ShaderD3D11::GetShaderByteData() const
{
	return _shaderBlob->GetBufferPointer();
}

size_t ShaderD3D11::GetShaderByteSize() const
{
	return _shaderBlob->GetBufferSize();
}
