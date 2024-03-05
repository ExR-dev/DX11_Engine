#include "Entity.h"

#include "ErrMsg.h"


Entity::Entity(const UINT id, const DirectX::BoundingBox &bounds)
{
	_entityID = id;
	_bounds = bounds;
}

bool Entity::Initialize(ID3D11Device *device, const UINT inputLayoutID, const UINT meshID, const UINT vsID, const UINT psID, const UINT texID)
{
	if (_isInitialized)
	{
		ErrMsg("Entity is already initialized!");
		return false;
	}

	if (!_transform.Initialize(device))
	{
		ErrMsg("Failed to initialize entity transform!");
		return false;
	}

	_inputLayoutID = inputLayoutID;
	_meshID = meshID;
	_vsID = vsID;
	_psID = psID;
	_texID = texID;

	_isInitialized = true;
	return true;
}

bool Entity::IsInitialized() const { return _isInitialized; }


void Entity::SetInputLayout(const UINT id)	{ _inputLayoutID = id;	}
void Entity::SetMesh(const UINT id)			{ _meshID = id;			}
void Entity::SetVertexShader(const UINT id)	{ _vsID = id;			}
void Entity::SetPixelShader(const UINT id)	{ _psID = id;			}
void Entity::SetTexture(const UINT id)		{ _texID = id;			}

Transform *Entity::GetTransform() { return &_transform; }

UINT Entity::GetID() const
{
	return _entityID;
}

bool Entity::StoreBounds(BoundingBox& entityBounds)
{
	if (_recalculateBounds)
	{
		_bounds.Transform(_transformedBounds, _transform.GetWorldMatrix());
		_recalculateBounds = false;
	}

	entityBounds = _transformedBounds;
	return true;
}


bool Entity::Update(ID3D11DeviceContext *context, Time &time, const Input &input)
{
	if (!_isInitialized)
	{
		ErrMsg("Entity is not initialized!");
		return false;
	}

	_recalculateBounds |= _transform.GetDirty();
	if (!_transform.UpdateConstantBuffer(context))
	{
		ErrMsg("Failed to set world matrix buffer!");
		return false;
	}

	return true;
}

bool Entity::BindBuffers(ID3D11DeviceContext *context) const
{
	if (!_isInitialized)
	{
		ErrMsg("Entity is not initialized!");
		return false;
	}

	ID3D11Buffer *const wmBuffer = _transform.GetConstantBuffer();
	context->VSSetConstantBuffers(0, 1, &wmBuffer);

	return true;
}

bool Entity::Render(CameraD3D11 *camera)
{
	if (!_isInitialized)
	{
		ErrMsg("Entity is not initialized!");
		return false;
	}

	const ResourceGroup resources = {
		_meshID,
		_vsID,
		_psID,
		_texID,
		0,
		_inputLayoutID
	};

	const RenderInstance instance = {
		this,
		sizeof(Entity)
	};

	camera->QueueRenderInstance(resources, instance);
	return true;
}
