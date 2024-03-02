#pragma once

#include <vector>
#include <string>
#include <wrl.h>

#include "InputLayoutD3D11.h"
#include "ShaderD3D11.h"
#include "MeshD3D11.h"
#include "SamplerD3D11.h"
#include "ShaderResourceTextureD3D11.h"


using Microsoft::WRL::ComPtr;

constexpr UINT CONTENT_LOAD_ERROR = 0xFFFFFFFF;


struct Mesh
{
	std::string name;
	UINT id;
	MeshD3D11 data;


	Mesh(std::string name, const UINT id) : name(std::move(name)), id(id) { }
	~Mesh() = default;

	Mesh(const Mesh &other) = delete;
	Mesh &operator=(const Mesh &other) = delete;
	Mesh(Mesh &&other) = delete;
	Mesh &operator=(Mesh &&other) = delete;
};

struct Shader
{
	std::string name;
	UINT id;
	ShaderD3D11 data;


	Shader(std::string name, const UINT id) : name(std::move(name)), id(id) { }
	~Shader() = default;

	Shader(const Shader &other) = delete;
	Shader &operator=(const Shader &other) = delete;
	Shader(Shader &&other) = delete;
	Shader &operator=(Shader &&other) = delete;
};

struct Texture
{
	std::string name;
	UINT id;
	ShaderResourceTextureD3D11 data;


	Texture(std::string name, const UINT id) : name(std::move(name)), id(id) { }
	~Texture() = default;

	Texture(const Texture &other) = delete;
	Texture &operator=(const Texture &other) = delete;
	Texture(Texture &&other) = delete;
	Texture &operator=(Texture &&other) = delete;
};

struct Sampler
{
	std::string name;
	UINT id;
	SamplerD3D11 data;


	Sampler(std::string name, const UINT id) : name(std::move(name)), id(id) { }
	~Sampler() = default;

	Sampler(const Sampler &other) = delete;
	Sampler &operator=(const Sampler &other) = delete;
	Sampler(Sampler &&other) = delete;
	Sampler &operator=(Sampler &&other) = delete;
};

struct InputLayout
{
	std::string name;
	UINT id;
	InputLayoutD3D11 data;


	InputLayout(std::string name, const UINT id) : name(std::move(name)), id(id) { }
	~InputLayout() = default;

	InputLayout(const InputLayout &other) = delete;
	InputLayout &operator=(const InputLayout &other) = delete;
	InputLayout(InputLayout &&other) = delete;
	InputLayout &operator=(InputLayout &&other) = delete;
};


class Content
{
private:
	// Pointers are used to avoid calling move constructors on the contents when the vectors are resized
	std::vector<Mesh *> _meshes; 
	std::vector<Shader *> _shaders;
	std::vector<Texture *> _textures;
	std::vector<Sampler *> _samplers;
	std::vector<InputLayout *> _inputLayouts;

public:
	Content();
	~Content();

	Content(const Content &other) = delete;
	Content &operator=(const Content &other) = delete;
	Content(Content &&other) = delete;
	Content &operator=(Content &&other) = delete;


	UINT AddMesh(ID3D11Device *device, const std::string &name, const MeshData &meshData);
	UINT AddMesh(ID3D11Device *device, const std::string &name, const char *path);

	UINT AddShader(ID3D11Device *device, const std::string &name, ShaderType shaderType, const void *dataPtr, size_t dataSize);
	UINT AddShader(ID3D11Device *device, const std::string &name, ShaderType shaderType, const char *path);

	UINT AddTexture(ID3D11Device *device, const std::string &name, UINT width, UINT height, const void *dataPtr);
	UINT AddTexture(ID3D11Device *device, const std::string &name, const char *path);

	UINT AddSampler(ID3D11Device *device, const std::string &name, D3D11_TEXTURE_ADDRESS_MODE adressMode,
		const std::optional<std::array<float, 4>> &borderColors = std::nullopt);

	UINT AddInputLayout(ID3D11Device *device, const std::string &name, const std::vector<Semantic> &semantics, 
		const void *vsByteData, size_t vsByteSize);
	UINT AddInputLayout(ID3D11Device *device, const std::string &name, const std::vector<Semantic> &semantics, UINT vShaderID);


	[[nodiscard]] UINT GetMeshCount() const;
	[[nodiscard]] UINT GetTextureCount() const;


	[[nodiscard]] UINT GetMeshID(const std::string &name) const;
	[[nodiscard]] MeshD3D11 *GetMesh(const std::string &name) const;
	[[nodiscard]] MeshD3D11 *GetMesh(UINT id) const;

	[[nodiscard]] UINT GetShaderID(const std::string &name) const;
	[[nodiscard]] ShaderD3D11 *GetShader(const std::string &name) const;
	[[nodiscard]] ShaderD3D11 *GetShader(UINT id) const;

	[[nodiscard]] UINT GetTextureID(const std::string &name) const;
	[[nodiscard]] ShaderResourceTextureD3D11 *GetTexture(const std::string &name) const;
	[[nodiscard]] ShaderResourceTextureD3D11 *GetTexture(UINT id) const;

	[[nodiscard]] UINT GetSamplerID(const std::string &name) const;
	[[nodiscard]] SamplerD3D11 *GetSampler(const std::string &name) const;
	[[nodiscard]] SamplerD3D11 *GetSampler(UINT id) const;

	[[nodiscard]] UINT GetInputLayoutID(const std::string &name) const;
	[[nodiscard]] InputLayoutD3D11 *GetInputLayout(const std::string &name) const;
	[[nodiscard]] InputLayoutD3D11 *GetInputLayout(UINT id) const;
};