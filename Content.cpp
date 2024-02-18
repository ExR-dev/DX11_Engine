#include "Content.h"
#include "ContentLoader.h"

#include "ErrMsg.h"


Content::Content()
{

}

Content::~Content()
{
	for (const Mesh *mesh : _meshes)
		delete mesh;

	for (const Shader *shader : _shaders)
		delete shader;

	for (const Texture *texture : _textures)
		delete texture;
}


UINT Content::AddMesh(ID3D11Device *device, const std::string &name, const MeshData &meshData)
{
	const UINT id = (UINT)_meshes.size();
	for (UINT i = 0; i < id; i++)
	{
		if (_meshes.at(i)->name == name)
			return i;
	}

	Mesh *addedMesh = new Mesh(name, id);
	if (!addedMesh->data.Initialize(device, meshData))
	{
		ErrMsg("Failed to initialize added mesh!");
		delete addedMesh;
		return CONTENT_LOAD_ERROR;
	}
	_meshes.push_back(addedMesh);

	return id;
}

UINT Content::AddMesh(ID3D11Device *device, const std::string &name, const char *path)
{
	const UINT id = static_cast<UINT>(_meshes.size());
	for (UINT i = 0; i < id; i++)
	{
		if (_meshes.at(i)->name == name)
			return i;
	}

	MeshData meshData = { };
	if (!LoadMeshFromFile(path, meshData))
	{
		ErrMsg("Failed to load mesh from file!");
		return CONTENT_LOAD_ERROR;
	}

	Mesh *addedMesh = new Mesh(name, id);
	if (!addedMesh->data.Initialize(device, meshData))
	{
		ErrMsg("Failed to initialize added mesh!");
		delete addedMesh;
		return CONTENT_LOAD_ERROR;
	}
	_meshes.push_back(addedMesh);

	return id;
}


UINT Content::AddShader(ID3D11Device *device, const std::string &name, const ShaderType shaderType, const void *dataPtr, const size_t dataSize)
{
	const UINT id = (UINT)_shaders.size();
	for (UINT i = 0; i < id; i++)
	{
		if (_shaders.at(i)->name == name)
			return i;
	}

	Shader *addedShader = new Shader(name, id);
	if (!addedShader->data.Initialize(device, shaderType, dataPtr, dataSize))
	{
		ErrMsg("Failed to initialize added shader!");
		delete addedShader;
		return CONTENT_LOAD_ERROR;
	}
	_shaders.push_back(addedShader);

	return id;
}

UINT Content::AddShader(ID3D11Device *device, const std::string &name, const ShaderType shaderType, const char *path)
{
	const UINT id = (UINT)_shaders.size();
	for (UINT i = 0; i < id; i++)
	{
		if (_shaders.at(i)->name == name)
			return i;
	}

	Shader *addedShader = new Shader(name, id);
	if (!addedShader->data.Initialize(device, shaderType, path))
	{
		ErrMsg("Failed to initialize added shader!");
		delete addedShader;
		return CONTENT_LOAD_ERROR;
	}
	_shaders.push_back(addedShader);

	return id;
}


UINT Content::AddTexture(ID3D11Device *device, const std::string &name, UINT width, UINT height, const void *dataPtr)
{
	const UINT id = (UINT)_textures.size();
	for (UINT i = 0; i < id; i++)
	{
		if (_textures.at(i)->name == name)
			return i;
	}

	Texture *addedTexture = new Texture(name, id);
	if (!addedTexture->data.Initialize(device, width, height, dataPtr))
	{
		ErrMsg("Failed to initialize added texture!");
		delete addedTexture;
		return CONTENT_LOAD_ERROR;
	}
	_textures.push_back(addedTexture);

	return id;
}

UINT Content::AddTexture(ID3D11Device *device, const std::string &name, const char *path)
{
	const UINT id = (UINT)_textures.size();
	for (UINT i = 0; i < id; i++)
	{
		if (_textures.at(i)->name == name)
			return i;
	}

	UINT width, height;
	std::vector<unsigned char> texData;

	if (!LoadTextureFromFile(path, width, height, texData))
	{
		ErrMsg("Failed to load texture from file!");
		return CONTENT_LOAD_ERROR;
	}

	Texture *addedTexture = new Texture(name, id);
	if (!addedTexture->data.Initialize(device, width, height, texData.data()))
	{
		ErrMsg("Failed to initialize added texture!");
		delete addedTexture;
		return CONTENT_LOAD_ERROR;
	}
	_textures.push_back(addedTexture);

	return id;
}


MeshD3D11 *Content::GetMesh(const std::string &name) const
{
	const UINT count = (UINT)_meshes.size();

	for (UINT i = 0; i < count; i++)
	{
		if (_meshes.at(i)->name == name)
			return &_meshes.at(i)->data;
	}

	return nullptr;
}

MeshD3D11 *Content::GetMesh(const UINT id) const
{
	if (id == CONTENT_LOAD_ERROR)
		return nullptr;
	if (_meshes.size() <= id)
		return nullptr;

	return &_meshes.at(id)->data;
}


ShaderD3D11 *Content::GetShader(const std::string &name) const
{
	const UINT count = (UINT)_shaders.size();

	for (UINT i = 0; i < count; i++)
	{
		if (_shaders.at(i)->name == name)
			return &_shaders.at(i)->data;
	}

	return nullptr;
}

ShaderD3D11 *Content::GetShader(const UINT id) const
{
	if (id == CONTENT_LOAD_ERROR)
		return nullptr;
	if (_shaders.size() <= id)
		return nullptr;

	return &_shaders.at(id)->data;
}


ShaderResourceTextureD3D11 *Content::GetTexture(const std::string &name) const
{
	const UINT count = (UINT)_textures.size();

	for (UINT i = 0; i < count; i++)
	{
		if (_textures.at(i)->name == name)
			return &_textures.at(i)->data;
	}

	return nullptr;
}

ShaderResourceTextureD3D11 *Content::GetTexture(const UINT id) const
{
	if (id == CONTENT_LOAD_ERROR)
		return nullptr;
	if (_textures.size() <= id)
		return nullptr;

	return &_textures.at(id)->data;
}
