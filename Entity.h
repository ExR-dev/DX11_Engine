#pragma once

#include "Transform.h"
#include "Time.h"
#include "Input.h"
#include "Graphics.h"


class Entity
{
private:
	UINT _entityID;
	bool _isInitialized = false;
	Transform _transform;

	UINT
		_inputLayoutID	= CONTENT_LOAD_ERROR,
		_meshID			= CONTENT_LOAD_ERROR,
		_vsID			= CONTENT_LOAD_ERROR,
		_psID			= CONTENT_LOAD_ERROR,
		_texID			= CONTENT_LOAD_ERROR;

public:
	explicit Entity(UINT id);
	~Entity() = default;
	Entity(const Entity &other) = delete;
	Entity &operator=(const Entity &other) = delete;
	Entity(Entity &&other) = delete;
	Entity &operator=(Entity &&other) = delete;

	[[nodiscard]] bool Initialize(ID3D11Device *device, UINT inputLayoutID, UINT meshID, UINT vsID, UINT psID, UINT texID);
	[[nodiscard]] bool IsInitialized() const;

	void SetInputLayout(UINT id);
	void SetMesh(UINT id);
	void SetVertexShader(UINT id);
	void SetPixelShader(UINT id);
	void SetTexture(UINT id);

	[[nodiscard]] Transform *GetTransform();

	[[nodiscard]] bool Update(ID3D11DeviceContext *context, const Time &time, const Input &input);
	[[nodiscard]] bool BindBuffers(ID3D11DeviceContext *context) const;
	[[nodiscard]] bool Render(Graphics *graphics);
};
