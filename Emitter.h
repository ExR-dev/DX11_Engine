#pragma once

#include "Entity.h"


struct EmitterData
{
	UINT particleCount, particleRate;
	XMFLOAT2A lifetimeRange, sizeRange, speedRange;
};


class Emitter final : Entity
{
private:
	EmitterData _settings = { };
	StructuredBufferD3D11 _particleBuffer;

public:
	explicit Emitter(UINT id, const DirectX::BoundingBox &bounds);

	[[nodiscard]] bool Initialize(ID3D11Device *device, const EmitterData *settings = nullptr);

	[[nodiscard]] EntityType GetType() const override;

	[[nodiscard]] bool Update(ID3D11DeviceContext *context, Time &time, const Input &input) override;
	[[nodiscard]] bool BindBuffers(ID3D11DeviceContext *context) const override;
	[[nodiscard]] bool Render(CameraD3D11 *camera) override;

	[[nodiscard]] bool UpdateParticles(ID3D11DeviceContext *context) const;
	[[nodiscard]] bool PerformDrawCall(ID3D11DeviceContext *context) const;
};
