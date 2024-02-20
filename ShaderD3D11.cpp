#include "ShaderD3D11.h"

#include <d3dcompiler.h>
#include <fstream>
#include <string>
#include <iostream>

#include "ErrMsg.h"


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


bool ShaderD3D11::Initialize(ID3D11Device *device, const ShaderType shaderType, const void *dataPtr, const size_t dataSize)
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
				ErrMsg("Failed to create vertex shader!");
				return false;
			}
			break;

		case ShaderType::HULL_SHADER:
			if (FAILED(device->CreateHullShader(
					shaderData, dataSize,
					nullptr, &_shader.hull)))
			{
				ErrMsg("Failed to create hull shader!");
				return false;
			}
			break;

		case ShaderType::DOMAIN_SHADER:
			if (FAILED(device->CreateDomainShader(
					shaderData, dataSize,
					nullptr, &_shader.domain)))
			{
				ErrMsg("Failed to create domain shader!");
				return false;
			}
			break;

		case ShaderType::GEOMETRY_SHADER:
			if (FAILED(device->CreateGeometryShader(
					shaderData, dataSize,
					nullptr, &_shader.geometry)))
			{
				ErrMsg("Failed to create geometry shader!");
				return false;
			}
			break;

		case ShaderType::PIXEL_SHADER:
			if (FAILED(device->CreatePixelShader(
					shaderData, dataSize,
					nullptr, &_shader.pixel)))
			{
				ErrMsg("Failed to create pixel shader!");
				return false;
			}
			break;

		case ShaderType::COMPUTE_SHADER:
			if (FAILED(device->CreateComputeShader(
					shaderData, dataSize,
					nullptr, &_shader.compute)))
			{
				ErrMsg("Failed to create compute shader!");
				return false;
			}
			break;
	}
	
	return true;
}

bool ShaderD3D11::Initialize(ID3D11Device *device, const ShaderType shaderType, const char *csoPath)
{
	std::string shaderFileData;
	std::ifstream reader;

	reader.open(csoPath, std::ios::binary | std::ios::ate);
	if (!reader.is_open())
	{
		ErrMsg("Failed to open shader file!");
		return false;
	}

	reader.seekg(0, std::ios::end);
	shaderFileData.reserve(static_cast<unsigned int>(reader.tellg()));
	reader.seekg(0, std::ios::beg);

	shaderFileData.assign(
		std::istreambuf_iterator<char>(reader),
		std::istreambuf_iterator<char>()
	);

	if (!Initialize(device, shaderType, shaderFileData.c_str(), shaderFileData.length()))
	{
		ErrMsg("Failed to initialize shader buffer!");
		return false;
	}

	shaderFileData.clear();
	reader.close();
	return true;
}


bool ShaderD3D11::BindShader(ID3D11DeviceContext *context) const
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

	return true;
}


const void *ShaderD3D11::GetShaderByteData() const
{
	return _shaderBlob->GetBufferPointer();
}

size_t ShaderD3D11::GetShaderByteSize() const
{
	return _shaderBlob->GetBufferSize();
}

ShaderType ShaderD3D11::GetShaderType() const
{
	return _type;
}
