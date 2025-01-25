#include "Object.h"
#include "ErrMsg.h"


Object::Object(const UINT id, const DirectX::BoundingOrientedBox &bounds) : Entity(id, bounds)
{
	_material = { CONTENT_LOAD_ERROR, CONTENT_LOAD_ERROR, CONTENT_LOAD_ERROR, CONTENT_LOAD_ERROR, CONTENT_LOAD_ERROR, CONTENT_LOAD_ERROR };
}

bool Object::Initialize(ID3D11Device *device, const std::string &name, const UINT meshID, const Material material, const bool isTransparent)
{
	if (!Entity::Initialize(device, name))
	{
		ErrMsg("Failed to initialize object!");
		return false;
	}

	_meshID = meshID;
	_material = material;
	_isTransparent = isTransparent;

	MaterialProperties materialProperties = { };
	materialProperties.sampleNormal = _material.normalID != CONTENT_LOAD_ERROR;
	materialProperties.sampleSpecular = _material.specularID != CONTENT_LOAD_ERROR;
	materialProperties.sampleReflection = _material.reflectiveID != CONTENT_LOAD_ERROR;
	materialProperties.sampleAmbient = _material.ambientID != CONTENT_LOAD_ERROR;

	if (!_materialBuffer.Initialize(device, sizeof(MaterialProperties), &materialProperties))
	{
		ErrMsg("Failed to initialize material buffer!");
		return false;
	}

	DirectX::SimpleMath::Vector3 worldPos = _transform.GetPosition(World);
	DirectX::SimpleMath::Vector4 paddedPos = { worldPos.x, worldPos.y, worldPos.z, 0 };

	if (!_posBuffer.Initialize(device, sizeof(DirectX::SimpleMath::Vector4), &paddedPos))
	{
		ErrMsg("Failed to initialize position buffer!");
		return false;
	}

	return true;
}


EntityType Object::GetType() const		{ return EntityType::OBJECT; }
UINT Object::GetMeshID() const			{ return _meshID; }
Material Object::GetMaterial() const	{ return _material; }

void Object::SetMeshID(const UINT id)				{ _meshID = id;	}
void Object::SetMaterial(const Material material)	{ _material = material;	}


bool Object::Update(ID3D11DeviceContext *context, Time &time, const Input &input)
{
	bool updatePosBuffer = _transform.IsDirty();

	if (!InternalUpdate(context))
	{
		ErrMsg("Failed to update object!");
		return false;
	}

	if (updatePosBuffer)
	{
		DirectX::BoundingOrientedBox worldSpaceBounds;
		StoreBounds(worldSpaceBounds);
		const DirectX::SimpleMath::Vector4 center = { worldSpaceBounds.Center.x, worldSpaceBounds.Center.y, worldSpaceBounds.Center.z, 0 };

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
		_material,
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
