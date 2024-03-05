#pragma once

#include "Transform.h"
#include "Time.h"
#include "Input.h"
#include "Graphics.h"


class IEntity
{
	public:
	virtual ~IEntity() = default;

	virtual Transform *GetTransform() = 0;
	virtual bool StoreBounds(DirectX::BoundingBox &entityBounds) = 0;

	virtual bool Update(ID3D11DeviceContext *context, Time &time, const Input &input) = 0;
	virtual bool BindBuffers(ID3D11DeviceContext *context) const = 0;
	virtual bool Render(CameraD3D11 *camera) = 0;
};


class Entity final : IEntity
{
private:
	UINT _entityID;
	bool _isInitialized = false;
	Transform _transform;

	DirectX::BoundingBox _bounds;
	DirectX::BoundingBox _transformedBounds;
	bool _recalculateBounds = true;

	UINT
		_inputLayoutID	= CONTENT_LOAD_ERROR,
		_meshID			= CONTENT_LOAD_ERROR,
		_vsID			= CONTENT_LOAD_ERROR,
		_psID			= CONTENT_LOAD_ERROR,
		_texID			= CONTENT_LOAD_ERROR;

public:
	explicit Entity(UINT id, const DirectX::BoundingBox &bounds);
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

	[[nodiscard]] Transform *GetTransform() override;
	[[nodiscard]] UINT GetID() const;

	[[nodiscard]] bool StoreBounds(DirectX::BoundingBox &entityBounds) override;

	[[nodiscard]] bool Update(ID3D11DeviceContext *context, Time &time, const Input &input) override;
	[[nodiscard]] bool BindBuffers(ID3D11DeviceContext *context) const override;
	[[nodiscard]] bool Render(CameraD3D11 *camera) override;
};
