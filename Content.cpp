#include "Content.h"
#include "ContentLoader.h"


Content::Content()
{

}

Content::~Content()
{

}


UINT Content::AddMesh(ID3D11Device *device, const std::string &name, const MeshData &meshData)
{
	const UINT id = (UINT)_meshes.size();
	for (UINT i = 0; i < id; i++)
	{
		if (_meshes.at(i).name == name)
			return i;
	}

	_meshes.emplace_back(name, id);
	_meshes.at(id).data.Initialize(device, meshData);
	return id;
}

UINT Content::AddMesh(ID3D11Device *device, const std::string &name, const char *path)
{
	MeshData meshData = { };
	if (!LoadMeshFromFile(path, meshData))
		return 0;

	const UINT id = (UINT)_meshes.size();
	for (UINT i = 0; i < id; i++)
	{
		if (_meshes.at(i).name == name)
			return i;
	}

	_meshes.emplace_back(name, id);
	_meshes.at(id).data.Initialize(device, meshData);
	return id;
}


UINT Content::AddShader(ID3D11Device *device, const std::string &name, const ShaderType shaderType, const void *dataPtr, const size_t dataSize)
{
	const UINT id = (UINT)_shaders.size();
	for (UINT i = 0; i < id; i++)
	{
		if (_shaders.at(i).name == name)
			return i;
	}

	_shaders.emplace_back(name, id);
	_shaders.at(id).data.Initialize(device, shaderType, dataPtr, dataSize);
	return id;
}

UINT Content::AddShader(ID3D11Device *device, const std::string &name, const ShaderType shaderType, const char *path)
{
	const UINT id = (UINT)_shaders.size();
	for (UINT i = 0; i < id; i++)
	{
		if (_shaders.at(i).name == name)
			return i;
	}

	_shaders.emplace_back(name, id);
	_shaders.at(id).data.Initialize(device, shaderType, path);
	return id;
}


MeshD3D11 *Content::GetMesh(const std::string &name)
{
	const UINT count = (UINT)_meshes.size();

	for (UINT i = 0; i < count; i++)
	{
		if (_meshes.at(i).name == name)
			return &_meshes.at(i).data;
	}

	return nullptr;
}

MeshD3D11 *Content::GetMesh(const UINT id)
{
	if (_meshes.size() <= id)
		return nullptr;

	return &_meshes.at(id).data;
}


ShaderD3D11 *Content::GetShader(const std::string &name)
{
	const UINT count = (UINT)_shaders.size();

	for (UINT i = 0; i < count; i++)
	{
		if (_shaders.at(i).name == name)
			return &_shaders.at(i).data;
	}

	return nullptr;
}

ShaderD3D11 *Content::GetShader(const UINT id)
{
	if (_shaders.size() <= id)
		return nullptr;

	return &_shaders.at(id).data;
}
