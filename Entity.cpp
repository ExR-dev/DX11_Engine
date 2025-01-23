#include "Entity.h"

#include "ErrMsg.h"


Entity::Entity(const UINT id, const DirectX::BoundingOrientedBox &bounds)
{
	_entityID = id;
	_bounds = bounds;
}

Entity::~Entity()
{
	for (auto& child : _children)
	{
		if (child != nullptr)
			child->SetParent(nullptr);
	}
	_children.clear();

	if (_parent)
		_parent->RemoveChild(this);
}


void Entity::AddChild(Entity *child, bool keepWorldTransform)
{
	if (!child)
		return;

	if (!_children.empty())
	{
		auto it = std::find(_children.begin(), _children.end(), child);

		if (it != _children.end())
			return;
	}

	_children.push_back(child);
	child->_transform.SetParent(&_transform, keepWorldTransform);
}

void Entity::RemoveChild(Entity *child, bool keepWorldTransform)
{
	if (!child)
		return;

	if (_children.empty())
		return;

	auto it = std::find(_children.begin(), _children.end(), child);

	if (it != _children.end())
		_children.erase(it);

	child->_transform.SetParent(nullptr, keepWorldTransform);
}


bool Entity::Initialize(ID3D11Device *device, const std::string &name)
{
	if (_isInitialized)
	{
		ErrMsg("Entity is already initialized!");
		return false;
	}

	SetName(name);

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

void Entity::SetDirty()
{
	for (auto &child : _children)
		child->SetDirty();

	_recalculateBounds = true;
	_transform.SetDirty();
}



void Entity::SetParent(Entity *parent, bool keepWorldTransform)
{
	keepWorldTransform = false; // TODO: Fix bug that causes crash after deleting child entities with keepWorldTransform = true

	if (_parent == parent)
		return;

	if (_parent)
		_parent->RemoveChild(this, keepWorldTransform);

	_parent = parent;

	/*if (parent)
		parent->AddChild(this);
	
	_transform.SetParent(parent ? parent->GetTransform() : nullptr, keepWorldTransform);*/

	if (parent)
		parent->AddChild(this, keepWorldTransform);
	else
		_transform.SetParent(nullptr, keepWorldTransform);

	SetDirty();
}

Entity *Entity::GetParent()
{
	return _parent;
}

const std::vector<Entity *> *Entity::GetChildren()
{
	return &_children;
}


UINT Entity::GetID() const
{
	return _entityID;
}

void Entity::SetName(const std::string &name)
{
	_name.assign(name);
}

const std::string Entity::GetName() const
{
	return _name;
}

Transform *Entity::GetTransform()
{
	return &_transform;
}


void Entity::StoreBounds(DirectX::BoundingOrientedBox &entityBounds)
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

	if (_transform.GetDirty())
	{
		SetDirty();
	}

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
