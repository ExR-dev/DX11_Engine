#include "Object.h"

#include "ErrMsg.h"


Object::Object(const UINT id, const DirectX::BoundingBox &bounds) : Entity(id, bounds)
{

}

bool Object::Initialize(ID3D11Device *device, const std::string &name,
	const UINT meshID, const UINT texID, 
	const UINT normalID, const UINT specularID,
	const UINT reflectiveID, const UINT ambientID, 
	const UINT heightID, const bool isTransparent)
{
	if (!Entity::Initialize(device, name))
	{
		ErrMsg("Failed to initialize object!");
		return false;
	}

	_meshID = meshID;
	_texID = texID;
	_normalID = normalID;
	_specularID = specularID;
	_reflectiveID = reflectiveID;
	_ambientID = ambientID;
	_heightID = heightID;
	_isTransparent = isTransparent;

	MaterialProperties materialProperties = { };
	materialProperties.sampleNormal = _normalID != CONTENT_LOAD_ERROR;
	materialProperties.sampleSpecular = _specularID != CONTENT_LOAD_ERROR;
	materialProperties.sampleReflection = _reflectiveID != CONTENT_LOAD_ERROR;
	materialProperties.sampleAmbient = _ambientID != CONTENT_LOAD_ERROR;

	if (!_materialBuffer.Initialize(device, sizeof(MaterialProperties), &materialProperties))
	{
		ErrMsg("Failed to initialize material buffer!");
		return false;
	}


	if (!_posBuffer.Initialize(device, sizeof(DirectX::XMFLOAT4A), &_transform.GetPosition()))
	{
		ErrMsg("Failed to initialize position buffer!");
		return false;
	}

	return true;
}


EntityType Object::GetType() const				{ return EntityType::OBJECT; }
UINT Object::GetMeshID(const UINT id) const		{ return _meshID; }
UINT Object::GetTextureID(const UINT id) const	{ return _texID; }

void Object::SetMesh(const UINT id)		{ _meshID = id;	}
void Object::SetTexture(const UINT id)	{ _texID = id;	}


bool Object::Update(ID3D11DeviceContext *context, Time &time, const Input &input)
{
	bool updatePosBuffer = _transform.GetDirty();

	if (!InternalUpdate(context))
	{
		ErrMsg("Failed to update object!");
		return false;
	}

	if (updatePosBuffer)
	{
		DirectX::BoundingBox worldSpaceBounds;
		StoreBounds(worldSpaceBounds);
		const DirectX::XMFLOAT4A center = { worldSpaceBounds.Center.x, worldSpaceBounds.Center.y, worldSpaceBounds.Center.z, 0.0f };

		if (!_posBuffer.UpdateBuffer(context, &center))
		{
			ErrMsg("Failed to update position buffer!");
			return false;
		}
	}

	return true;
}

bool Object::BindBuffers(ID3D11DeviceContext *context) const
{
	if (!InternalBindBuffers(context))
	{
		ErrMsg("Failed to bind object buffers!");
		return false;
	}

	ID3D11Buffer *const materialBuffer = _materialBuffer.GetBuffer();
	context->PSSetConstantBuffers(2, 1, &materialBuffer);

	ID3D11Buffer *const posBuffer = _posBuffer.GetBuffer();
	context->HSSetConstantBuffers(0, 1, &posBuffer);

	return true;
}

bool Object::Render(CameraD3D11 *camera)
{
	if (!InternalRender(camera))
	{
		ErrMsg("Failed to render object!");
		return false;
	}

	const ResourceGroup resources = {
		_meshID,
		_texID,
		_normalID,
		_specularID,
		_reflectiveID,
		_ambientID,
		_heightID,
	};

	const RenderInstance instance = {
		this,
		sizeof(Object)
	};

	if (_isTransparent)
		camera->QueueTransparent(resources, instance);
	else
		camera->QueueGeometry(resources, instance);

	return true;
}
