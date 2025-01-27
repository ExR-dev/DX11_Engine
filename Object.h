#pragma once

#include "Entity.h"

class Object final : public Entity
{
private:
	UINT
		_meshID = CONTENT_NULL,
		_texID = CONTENT_NULL,
		_normalID = CONTENT_NULL,
		_specularID = CONTENT_NULL,
		_reflectiveID = CONTENT_NULL,
		_ambientID = CONTENT_NULL,
		_heightID = CONTENT_NULL;

	bool _isTransparent = false;

	ConstantBufferD3D11
		_materialBuffer,
		_posBuffer;

public:
	explicit Object(UINT id, const DirectX::BoundingOrientedBox &bounds);

	[[nodiscard]] bool Initialize(ID3D11Device *device, const std::string &name,
		UINT meshID, UINT texID, 
		UINT normalID, UINT specularID, 
		UINT reflectiveID, UINT ambientID, 
		UINT heightID, bool isTransparent = false);

	[[nodiscard]] EntityType GetType() const override;

	[[nodiscard]] UINT GetMeshID(UINT id) const;
	[[nodiscard]] UINT GetTextureID(UINT id) const;

	void SetMesh(UINT id);
	void SetTexture(UINT id);

	[[nodiscard]] bool Update(ID3D11DeviceContext *context, Time &time, const Input &input) override;
	[[nodiscard]] bool BindBuffers(ID3D11DeviceContext *context) const override;
	[[nodiscard]] bool Render(CameraD3D11 *camera) override;
};
