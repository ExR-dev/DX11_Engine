#pragma once

#include "Entity.h"


struct EmitterData
{
	UINT particleCount;
	UINT particleRate;
	float lifetime;
	float deltaTime;
};

struct Particle
{
	XMFLOAT3 position	= { 0, 0, 0 };
	XMFLOAT3 velocity	= { 0, 0, 0 };
	XMFLOAT4 color		= { 0, 0, 0, 1 };
	float size			= 1;
	float lifetime		= 0;
};


class Emitter final : Entity
{
private:
	EmitterData _emitterData = { };

	ConstantBufferD3D11 _emitterBuffer;
	StructuredBufferD3D11 _particleBuffer;

	UINT _texID = CONTENT_LOAD_ERROR;


public:
	explicit Emitter(UINT id, const DirectX::BoundingBox &bounds);

	[[nodiscard]] bool Initialize(ID3D11Device *device, const EmitterData &settings, UINT textureID);

	[[nodiscard]] EntityType GetType() const override;

	[[nodiscard]] UINT GetTextureID() const;

	[[nodiscard]] bool Update(ID3D11DeviceContext *context, Time &time, const Input &input) override;
	[[nodiscard]] bool BindBuffers(ID3D11DeviceContext *context) const override;
	[[nodiscard]] bool Render(CameraD3D11 *camera) override;

	[[nodiscard]] bool PerformDrawCall(ID3D11DeviceContext *context) const;
};
