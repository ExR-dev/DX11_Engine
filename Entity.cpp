#include "Entity.h"

#include "ErrMsg.h"


Entity::Entity()
{

}

Entity::~Entity()
{

}


bool Entity::Initialize(ID3D11Device *device, const UINT inputLayoutID, const UINT meshID, const UINT vsID, const UINT psID, const UINT texID)
{
	if (_isInitialized)
	{
		ErrMsg("Entity already initialized!");
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

	return true;
}

bool Entity::IsInitialized() const
{
	return _isInitialized;
}


bool Entity::Update(ID3D11DeviceContext *context, const Time &time)
{
	if (!_transform.UpdateConstantBuffer(context))
	{
		ErrMsg("Failed to set world matrix buffer!");
		return false;
	}

	return true;
}

bool Entity::Render(Graphics *graphics)
{
	const RenderInstance instance = {
		_inputLayoutID,
		_meshID,
		_vsID,
		_psID,
		_texID,
		this,
		sizeof(Entity)
	};

	if (!graphics->QueueRender(instance))
	{
		ErrMsg("Failed to queue entity for rendering!");
		return false;
	}

	return true;
}