#pragma once

#include <vector>
#include <string>

#include "ShaderD3D11.h"
#include "MeshD3D11.h"
#include "ShaderResourceTextureD3D11.h"


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


class Content
{
private:
	// Pointers are used to avoid calling move constructors on the contents when the vectors are resized
	std::vector<Mesh *> _meshes; 
	std::vector<Shader *> _shaders;
	std::vector<Texture *> _textures;


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


	[[nodiscard]] MeshD3D11 *GetMesh(const std::string &name) const;
	[[nodiscard]] MeshD3D11 *GetMesh(const UINT id) const;

	[[nodiscard]] ShaderD3D11 *GetShader(const std::string &name) const;
	[[nodiscard]] ShaderD3D11 *GetShader(const UINT id) const;

	[[nodiscard]] ShaderResourceTextureD3D11 *GetTexture(const std::string &name) const;
	[[nodiscard]] ShaderResourceTextureD3D11 *GetTexture(const UINT id) const;
};