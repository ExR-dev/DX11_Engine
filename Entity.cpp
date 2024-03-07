#include "Entity.h"

#include "ErrMsg.h"


Entity::Entity(const UINT id, const DirectX::BoundingBox &bounds)
{
	_entityID = id;
	_bounds = bounds;
}


bool Entity::Initialize(ID3D11Device *device)
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

	_isInitialized = true;
	return true;
}

bool Entity::IsInitialized() const
{
	return _isInitialized;
}


UINT Entity::GetID() const
{
	return _entityID;
}

Transform *Entity::GetTransform()
{
	return &_transform;
}


void Entity::StoreBounds(DirectX::BoundingBox &entityBounds)
{
	if (_recalculateBounds)
	{
		_bounds.Transform(_transformedBounds, _transform.GetWorldMatrix());
		_recalculateBounds = false;
	}

	entityBounds = _transformedBounds;
}


bool Entity::InternalUpdate(ID3D11DeviceContext *context)
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

bool Entity::InternalBindBuffers(ID3D11DeviceContext *context) const
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

bool Entity::InternalRender(CameraD3D11 *camera)
{
	if (!_isInitialized)
	{
		ErrMsg("Entity is not initialized!");
		return false;
	}

	return true;
}
