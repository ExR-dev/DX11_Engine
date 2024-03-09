#pragma once

#include "Entity.h"


class Object final : Entity
{
private:
	UINT
		_meshID = CONTENT_LOAD_ERROR,
		_texID = CONTENT_LOAD_ERROR;

	bool _isTransparent = false;

public:
	explicit Object(UINT id, const DirectX::BoundingBox &bounds);

	[[nodiscard]] bool Initialize(ID3D11Device *device, UINT meshID, UINT texID, bool isTransparent = false);

	[[nodiscard]] EntityType GetType() const override;

	[[nodiscard]] UINT GetMeshID(UINT id) const;
	[[nodiscard]] UINT GetTextureID(UINT id) const;

	void SetMesh(UINT id);
	void SetTexture(UINT id);

	[[nodiscard]] bool Update(ID3D11DeviceContext *context, Time &time, const Input &input) override;
	[[nodiscard]] bool BindBuffers(ID3D11DeviceContext *context) const override;
	[[nodiscard]] bool Render(CameraD3D11 *camera) override;
};
