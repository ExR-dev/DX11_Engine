#pragma once

#include <vector>
#include <string>

#include "ToDistribute/ShaderD3D11.h"
#include "ToDistribute/MeshD3D11.h"


constexpr UINT LOAD_ERROR = 0xFFFFFFFF;


struct Mesh
{
	std::string name;
	UINT id;
	MeshD3D11 data;

	Mesh(std::string name, const UINT id) : name(std::move(name)), id(id) { }
	Mesh(const Mesh &other) = delete;
	Mesh &operator=(const Mesh &other) = delete;
	Mesh(Mesh &&other) noexcept : name(std::move(other.name)), id(other.id) { }
	Mesh &operator=(Mesh &&other) = delete;
};

struct Shader
{
	std::string name;
	UINT id;
	ShaderD3D11 data;

	Shader(std::string name, const UINT id) : name(std::move(name)), id(id) { }
	Shader(const Shader &other) = delete;
	Shader &operator=(const Shader &other) = delete;
	Shader(Shader &&other) noexcept : name(std::move(other.name)), id(other.id) { }
	Shader &operator=(Shader &&other) = delete;
};

class Content
{
private:
	std::vector<Mesh> _meshes;
	std::vector<Shader> _shaders;

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

	MeshD3D11 *GetMesh(const std::string &name);
	MeshD3D11 *GetMesh(const UINT id);

	ShaderD3D11 *GetShader(const std::string &name);
	ShaderD3D11 *GetShader(const UINT id);
};