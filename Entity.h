#pragma once

#include "Transform.h"
#include "Time.h"
#include "Graphics.h"


class Entity
{
private:
	bool _isInitialized = false;
	Transform _transform;

	UINT _inputLayoutID = CONTENT_LOAD_ERROR;
	UINT _meshID		= CONTENT_LOAD_ERROR;
	UINT _vsID			= CONTENT_LOAD_ERROR;
	UINT _psID			= CONTENT_LOAD_ERROR;
	UINT _texID			= CONTENT_LOAD_ERROR;

public:
	Entity();
	~Entity();
	Entity(const Entity &other) = delete;
	Entity &operator=(const Entity &other) = delete;
	Entity(Entity &&other) = delete;
	Entity &operator=(Entity &&other) = delete;

	bool Initialize(ID3D11Device *device, UINT inputLayoutID, UINT meshID, UINT vsID, UINT psID, UINT texID);
	bool IsInitialized() const;

	bool Update(ID3D11DeviceContext *context, const Time &time);
	bool Render(Graphics *graphics);
};
