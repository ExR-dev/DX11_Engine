#pragma once

#include "Entity.h"
#include "Material.h"


class Object final : Entity
{
private:
	UINT _meshID = CONTENT_LOAD_ERROR;
	Material _material; // TODO: Convert to pointer, store all materials in Content.h.

	bool _isTransparent = false;

	ConstantBufferD3D11
		_materialBuffer,
		_posBuffer;

public:
	explicit Object(UINT id, const DirectX::BoundingOrientedBox &bounds);

	[[nodiscard]] bool Initialize(ID3D11Device *device, const std::string &name, UINT meshID, Material material, bool isTransparent = false);

	[[nodiscard]] EntityType GetType() const override;

	[[nodiscard]] UINT GetMeshID() const;
	[[nodiscard]] Material GetMaterial() const;

	void SetMeshID(UINT id);
	void SetMaterial(Material material);

	[[nodiscard]] bool Update(ID3D11DeviceContext *context, Time &time, const Input &input) override;
	[[nodiscard]] bool BindBuffers(ID3D11DeviceContext *context) const override;
	[[nodiscard]] bool Render(CameraD3D11 *camera) override;
};
