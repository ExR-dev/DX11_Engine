#include "Object.h"

#include "ErrMsg.h"


Object::Object(const UINT id, const DirectX::BoundingBox &bounds) : Entity(id, bounds)
{

}

bool Object::Initialize(ID3D11Device *device, const UINT meshID, const UINT texID)
{
	if (!Entity::Initialize(device))
	{
		ErrMsg("Failed to initialize object!");
		return false;
	}

	_meshID = meshID;
	_texID = texID;

	return true;
}


EntityType Object::GetType() const				{ return EntityType::OBJECT; }
UINT Object::GetMeshID(const UINT id) const		{ return _meshID; }
UINT Object::GetTextureID(const UINT id) const	{ return _texID; }

void Object::SetMesh(const UINT id)		{ _meshID = id;	}
void Object::SetTexture(const UINT id)	{ _texID = id;	}


bool Object::Update(ID3D11DeviceContext *context, Time &time, const Input &input)
{
	if (!InternalUpdate(context))
	{
		ErrMsg("Failed to update object!");
		return false;
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
	};

	const RenderInstance instance = {
		this,
		sizeof(Object)
	};

	camera->QueueRenderInstance(resources, instance);
	return true;
}