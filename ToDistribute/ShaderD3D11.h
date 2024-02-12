#pragma once

#include <d3d11_4.h>


enum class ShaderType
{
	VERTEX_SHADER	= 0,
	HULL_SHADER		= 1,
	DOMAIN_SHADER	= 2,
	GEOMETRY_SHADER = 3,
	PIXEL_SHADER	= 4,
	COMPUTE_SHADER	= 5,
};

class ShaderD3D11
{
private:

	ShaderType _type;

	union
	{
		ID3D11VertexShader* vertex = nullptr;
		ID3D11HullShader* hull;
		ID3D11DomainShader* domain;
		ID3D11GeometryShader* geometry;
		ID3D11PixelShader* pixel;
		ID3D11ComputeShader* compute;
	} _shader;

	ID3DBlob* _shaderBlob = nullptr;

public:
	ShaderD3D11() = default;
	~ShaderD3D11();

	ShaderD3D11(const ShaderD3D11& other) = delete;
	ShaderD3D11& operator=(const ShaderD3D11& other) = delete;
	ShaderD3D11(ShaderD3D11&& other) = delete;
	ShaderD3D11& operator=(ShaderD3D11&& other) = delete;

	void Initialize(ID3D11Device* device, ShaderType shaderType, const void* dataPtr, size_t dataSize);
	void Initialize(ID3D11Device* device, ShaderType shaderType, const char* csoPath);

	void BindShader(ID3D11DeviceContext* context) const;

	const void* GetShaderByteData() const;
	size_t GetShaderByteSize() const;
};