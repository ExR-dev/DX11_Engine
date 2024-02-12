#include "ShaderD3D11.h"

#include <d3dcompiler.h>
#include <fstream>
#include <string>
#include <iostream>


bool LoadShader(ID3D11Device *device, const ShaderType type, void *&shader, const std::string &filePath, std::string &byteCode)
{
	std::string shaderData;
	std::ifstream reader;

	reader.open(filePath, std::ios::binary | std::ios::ate);
	if (!reader.is_open())
	{
		std::cerr << "Could not open shader file!" << std::endl;
		return false;
	}

	reader.seekg(0, std::ios::end);
	shaderData.reserve(static_cast<unsigned int>(reader.tellg()));
	reader.seekg(0, std::ios::beg);

	shaderData.assign(
		std::istreambuf_iterator<char>(reader),
		std::istreambuf_iterator<char>()
	);

	switch (type)
	{
		case ShaderType::VERTEX_SHADER:
			if (FAILED(device->CreateVertexShader(
				shaderData.c_str(), shaderData.length(),
				nullptr, &((ID3D11VertexShader *&)shader))))
			{
				std::cerr << "Failed to create vertex shader!" << std::endl;
				return false;
			}
			break;

		case ShaderType::PIXEL_SHADER:
			if (FAILED(device->CreatePixelShader(
				shaderData.c_str(), shaderData.length(),
				nullptr, &((ID3D11PixelShader *&)shader))))
			{
				std::cerr << "Failed to create pixel shader!" << std::endl;
				return false;
			}
			break;

		default:
			std::cerr << "Shader type not implemented!" << std::endl;
			reader.close();
			return false;
	}

	byteCode = shaderData;
	shaderData.clear();
	reader.close();

	return true;
}


ShaderD3D11::ShaderD3D11(ID3D11Device *device, ShaderType shaderType, const void *dataPtr, size_t dataSize)
{

}

ShaderD3D11::ShaderD3D11(ID3D11Device *device, ShaderType shaderType, const char *csoPath)
{

}

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


void ShaderD3D11::Initialize(ID3D11Device *device, ShaderType shaderType, const void *dataPtr, size_t dataSize)
{

	//D3DCreateBlob(dataSize, &_shaderBlob);
}

void ShaderD3D11::Initialize(ID3D11Device *device, ShaderType shaderType, const char *csoPath)
{

	//D3DCreateBlob(dataSize, &_shaderBlob);
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
