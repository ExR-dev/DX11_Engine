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

	for (const TextureMap *textureMap : _textureMaps)
		delete textureMap;

	for (const Sampler *sampler : _samplers)
		delete sampler;

	for (const InputLayout *inputLayout : _inputLayouts)
		delete inputLayout;
}


UINT Content::AddMesh(ID3D11Device *device, const std::string &name, const MeshData &meshData)
{
	const UINT id = static_cast<UINT>(_meshes.size());
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

	/*std::string meshDebugPath = path;
	meshDebugPath.replace(meshDebugPath.find_last_of('.'), 4, "_DEBUG.txt");

	if (!WriteMeshToFile(meshDebugPath.c_str(), meshData))
	{
		ErrMsg("Failed to write mesh to file!");
		return CONTENT_LOAD_ERROR;
	}*/

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
	const UINT id = static_cast<UINT>(_shaders.size());
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
	const UINT id = static_cast<UINT>(_shaders.size());
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


UINT Content::AddTexture(ID3D11Device *device, const std::string &name, const UINT width, const UINT height, const void *dataPtr)
{
	const UINT id = static_cast<UINT>(_textures.size());
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
	const UINT id = static_cast<UINT>(_textures.size());
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


UINT Content::AddTextureMap(ID3D11Device *device, const std::string &name, const TextureType mapType, const UINT width, const UINT height, const void *dataPtr)
{
	const UINT id = static_cast<UINT>(_textureMaps.size());
	for (UINT i = 0; i < id; i++)
	{
		if (_textureMaps.at(i)->name == name)
			return i;
	}

	D3D11_TEXTURE2D_DESC textureDesc = { };
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.ArraySize = 1;
	textureDesc.MipLevels = 1;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;

	D3D11_SUBRESOURCE_DATA srData = { };
	srData.pSysMem = dataPtr;
	srData.SysMemSlicePitch = 0;

	switch (mapType)
	{
		case TextureType::SPECULAR:
			textureDesc.Format = DXGI_FORMAT_R8_UNORM;
			srData.SysMemPitch = width * sizeof(unsigned char);
			break;

		default:
			textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			srData.SysMemPitch = width * sizeof(unsigned char) * 4;
			break;
	}

	TextureMap *addedTextureMap = new TextureMap(name, id);
	if (!addedTextureMap->data.Initialize(device, textureDesc, &srData))
	{
		ErrMsg("Failed to initialize added texture map!");
		delete addedTextureMap;
		return CONTENT_LOAD_ERROR;
	}
	_textureMaps.push_back(addedTextureMap);

	return id;
}

UINT Content::AddTextureMap(ID3D11Device *device, const std::string &name, const TextureType mapType, const char *path)
{
	const UINT id = static_cast<UINT>(_textureMaps.size());
	for (UINT i = 0; i < id; i++)
	{
		if (_textureMaps.at(i)->name == name)
			return i;
	}

	UINT width, height;
	std::vector<unsigned char> texData, texMapData;

	if (!LoadTextureFromFile(path, width, height, texData))
	{
		ErrMsg("Failed to load texture map from file!");
		return CONTENT_LOAD_ERROR;
	}

	D3D11_TEXTURE2D_DESC textureDesc = { };
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.ArraySize = 1;
	textureDesc.MipLevels = 1;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;

	D3D11_SUBRESOURCE_DATA srData = { };
	srData.SysMemSlicePitch = 0;

	switch (mapType)
	{
		case TextureType::SPECULAR:
			textureDesc.Format = DXGI_FORMAT_R8_UNORM;
			srData.SysMemPitch = width * sizeof(unsigned char);

			for (size_t i = 0; i < texData.size(); i += 4)
				texMapData.push_back(texData.at(i));
			break;

		default:
			textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			srData.SysMemPitch = width * sizeof(unsigned char) * 4;

			texMapData.insert(texMapData.end(), texData.begin(), texData.end());
			break;
	}

	srData.pSysMem = texMapData.data();

	TextureMap *addedTextureMap = new TextureMap(name, id);
	if (!addedTextureMap->data.Initialize(device, textureDesc, &srData))
	{
		ErrMsg("Failed to initialize added texture map!");
		delete addedTextureMap;
		return CONTENT_LOAD_ERROR;
	}
	_textureMaps.push_back(addedTextureMap);

	return id;
}


UINT Content::AddSampler(ID3D11Device *device, const std::string &name, const D3D11_TEXTURE_ADDRESS_MODE adressMode, 
	const std::optional<std::array<float, 4>> &borderColors)
{
	const UINT id = static_cast<UINT>(_samplers.size());
	for (UINT i = 0; i < id; i++)
	{
		if (_samplers.at(i)->name == name)
			return i;
	}

	Sampler *addedSampler = new Sampler(name, id);
	if (!addedSampler->data.Initialize(device, adressMode, borderColors))
	{
		ErrMsg("Failed to initialize added texture!");
		delete addedSampler;
		return CONTENT_LOAD_ERROR;
	}
	_samplers.push_back(addedSampler);

	return id;
}


UINT Content::AddInputLayout(ID3D11Device *device, const std::string &name, const std::vector<Semantic> &semantics, 
	const void *vsByteData, const size_t vsByteSize)
{
	const UINT id = static_cast<UINT>(_inputLayouts.size());
	for (UINT i = 0; i < id; i++)
	{
		if (_inputLayouts.at(i)->name == name)
			return i;
	}

	InputLayout *addedInputLayout = new InputLayout(name, id);
	for (const Semantic &semantic : semantics)
	{
		if (!addedInputLayout->data.AddInputElement(semantic))
		{
			ErrMsg(std::format("Failed to add element \"{}\" to input layout!", semantic.name));
			delete addedInputLayout;
			return CONTENT_LOAD_ERROR;
		}
	}

	if (!addedInputLayout->data.FinalizeInputLayout(device, vsByteData, vsByteSize))
	{
		ErrMsg("Failed to finalize added input layout!");
		delete addedInputLayout;
		return CONTENT_LOAD_ERROR;
	}
	_inputLayouts.push_back(addedInputLayout);

	return id;
}

UINT Content::AddInputLayout(ID3D11Device *device, const std::string &name, const std::vector<Semantic> &semantics, const UINT vShaderID)
{
	if (vShaderID == CONTENT_LOAD_ERROR)
	{
		ErrMsg("Failed to get vertex shader byte code, shader ID was CONTENT_LOAD_ERROR!");
		return CONTENT_LOAD_ERROR;
	}

	const ShaderD3D11 *vShader = GetShader(vShaderID);
	if (vShader == nullptr)
	{
		ErrMsg("Failed to get vertex shader byte code, shader ID returned nullptr!");
		return CONTENT_LOAD_ERROR;
	}

	if (vShader->GetShaderType() != ShaderType::VERTEX_SHADER)
	{
		ErrMsg(std::format("Failed to get vertex shader byte code, shader ID returned invalid type ({})!", (UINT)vShader->GetShaderType()));
		return CONTENT_LOAD_ERROR;
	}
	
	return AddInputLayout(device, name, semantics, vShader->GetShaderByteData(), vShader->GetShaderByteSize());
}


UINT Content::GetMeshCount() const
{
	return _meshes.size();
}

UINT Content::GetTextureCount() const
{
	return _textures.size();
}


UINT Content::GetMeshID(const std::string &name) const
{
	const UINT count = static_cast<UINT>(_meshes.size());

	for (UINT i = 0; i < count; i++)
	{
		if (_meshes.at(i)->name == name)
			return i;
	}

	return CONTENT_LOAD_ERROR;
}

MeshD3D11 *Content::GetMesh(const std::string &name) const
{
	const UINT count = static_cast<UINT>(_meshes.size());

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


UINT Content::GetShaderID(const std::string &name) const
{
	const UINT count = static_cast<UINT>(_shaders.size());

	for (UINT i = 0; i < count; i++)
	{
		if (_shaders.at(i)->name == name)
			return i;
	}

	return CONTENT_LOAD_ERROR;
}

ShaderD3D11 *Content::GetShader(const std::string &name) const
{
	const UINT count = static_cast<UINT>(_shaders.size());

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


UINT Content::GetTextureID(const std::string &name) const
{
	const UINT count = static_cast<UINT>(_textures.size());

	for (UINT i = 0; i < count; i++)
	{
		if (_textures.at(i)->name == name)
			return i;
	}

	return CONTENT_LOAD_ERROR;
}

ShaderResourceTextureD3D11 *Content::GetTexture(const std::string &name) const
{
	const UINT count = static_cast<UINT>(_textures.size());

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


UINT Content::GetTextureMapID(const std::string &name) const
{
	const UINT count = static_cast<UINT>(_textureMaps.size());

	for (UINT i = 0; i < count; i++)
	{
		if (_textureMaps.at(i)->name == name)
			return i;
	}

	return CONTENT_LOAD_ERROR;
}

ShaderResourceTextureD3D11 *Content::GetTextureMap(const std::string &name) const
{
	const UINT count = static_cast<UINT>(_textureMaps.size());

	for (UINT i = 0; i < count; i++)
	{
		if (_textureMaps.at(i)->name == name)
			return &_textureMaps.at(i)->data;
	}

	return nullptr;
}

ShaderResourceTextureD3D11 *Content::GetTextureMap(const UINT id) const
{
	if (id == CONTENT_LOAD_ERROR)
		return nullptr;
	if (_textureMaps.size() <= id)
		return nullptr;

	return &_textureMaps.at(id)->data;
}


UINT Content::GetSamplerID(const std::string &name) const
{
	const UINT count = static_cast<UINT>(_samplers.size());

	for (UINT i = 0; i < count; i++)
	{
		if (_samplers.at(i)->name == name)
			return i;
	}

	return CONTENT_LOAD_ERROR;
}

SamplerD3D11 *Content::GetSampler(const std::string &name) const
{
	const UINT count = static_cast<UINT>(_samplers.size());

	for (UINT i = 0; i < count; i++)
	{
		if (_samplers.at(i)->name == name)
			return &_samplers.at(i)->data;
	}

	return nullptr;
}

SamplerD3D11 *Content::GetSampler(const UINT id) const
{
	if (id == CONTENT_LOAD_ERROR)
		return nullptr;
	if (_samplers.size() <= id)
		return nullptr;

	return &_samplers.at(id)->data;
}


UINT Content::GetInputLayoutID(const std::string &name) const
{
	const UINT count = static_cast<UINT>(_inputLayouts.size());

	for (UINT i = 0; i < count; i++)
	{
		if (_inputLayouts.at(i)->name == name)
			return i;
	}

	return CONTENT_LOAD_ERROR;
}

InputLayoutD3D11 *Content::GetInputLayout(const std::string &name) const
{
	const UINT count = static_cast<UINT>(_inputLayouts.size());

	for (UINT i = 0; i < count; i++)
	{
		if (_inputLayouts.at(i)->name == name)
			return &_inputLayouts.at(i)->data;
	}

	return nullptr;
}

InputLayoutD3D11 *Content::GetInputLayout(const UINT id) const
{
	if (id == CONTENT_LOAD_ERROR)
		return nullptr;
	if (_inputLayouts.size() <= id)
		return nullptr;

	return &_inputLayouts.at(id)->data;
}
