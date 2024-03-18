#pragma once

#include "Entity.h"


class Object final : Entity
{
private:
	UINT
		_meshID = CONTENT_LOAD_ERROR,
		_texID = CONTENT_LOAD_ERROR,
		_normalID = CONTENT_LOAD_ERROR,
		_specularID = CONTENT_LOAD_ERROR,
		_reflectiveID = CONTENT_LOAD_ERROR;

	bool _isTransparent = false;

	ConstantBufferD3D11 _materialBuffer;

public:
	explicit Object(UINT id, const DirectX::BoundingBox &bounds);

	[[nodiscard]] bool Initialize(ID3D11Device *device, UINT meshID, UINT texID, 
		UINT normalID = CONTENT_LOAD_ERROR, UINT specularID = CONTENT_LOAD_ERROR, 
		UINT reflectiveID = CONTENT_LOAD_ERROR, bool isTransparent = false);

	[[nodiscard]] EntityType GetType() const override;

	[[nodiscard]] UINT GetMeshID(UINT id) const;
	[[nodiscard]] UINT GetTextureID(UINT id) const;

	void SetMesh(UINT id);
	void SetTexture(UINT id);

	[[nodiscard]] bool Update(ID3D11DeviceContext *context, Time &time, const Input &input) override;
	[[nodiscard]] bool BindBuffers(ID3D11DeviceContext *context) const override;
	[[nodiscard]] bool Render(CameraD3D11 *camera) override;
};
