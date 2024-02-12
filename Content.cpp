#include "Content.h"


Content::Content()
{

}

Content::~Content()
{

}


UINT Content::AddMesh(ID3D11Device *device, const std::string &name, const MeshData &meshInfo)
{
	const UINT id = _meshes.size();
	for (UINT i = 0; i < id; i++)
	{
		if (_meshes.at(i).name == name)
			return i;
	}

	_meshes.emplace_back(name, id);
	_meshes.at(id).data.Initialize(device, meshInfo);
	return id;
}

UINT Content::AddShader(ID3D11Device *device, const std::string &name, const ShaderType shaderType, const void *dataPtr, const size_t dataSize)
{
	const UINT id = _shaders.size();
	for (UINT i = 0; i < id; i++)
	{
		if (_shaders.at(i).name == name)
			return i;
	}

	_shaders.emplace_back(name, id);
	_shaders.at(id).data.Initialize(device, shaderType, dataPtr, dataSize);
	return id;
}

UINT Content::AddShader(ID3D11Device *device, const std::string &name, const ShaderType shaderType, const char *csoPath)
{
	const UINT id = _shaders.size();
	for (UINT i = 0; i < id; i++)
	{
		if (_shaders.at(i).name == name)
			return i;
	}

	_shaders.emplace_back(name, id);
	_shaders.at(id).data.Initialize(device, shaderType, csoPath);
	return id;
}


MeshD3D11 *Content::GetMesh(const std::string &name)
{
	const UINT count = _meshes.size();

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
	const UINT count = _shaders.size();

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
