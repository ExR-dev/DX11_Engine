#pragma once

#include "Entity.h"


struct EmitterData
{
	UINT particleCount, particleRate;
	float lifetime, padding;
	//XMFLOAT2A sizeRange, speedRange;
};

struct Particle
{
	XMFLOAT3
		position	= { 0, 0, 0 },
		velocity	= { 0, 0, 0 },
		color		= { 0, 0, 0 };
	float
		size		= 1,
		lifetime	= 0,
		padding		= 0;
};


class Emitter final : Entity
{
private:
	EmitterData _settings = { };

	ConstantBufferD3D11 _emitterBuffer;
	ConstantBufferD3D11 _timeBuffer;
	StructuredBufferD3D11 _particleBuffer;


public:
	explicit Emitter(UINT id, const DirectX::BoundingBox &bounds);

	[[nodiscard]] bool Initialize(ID3D11Device *device, const EmitterData &settings);

	[[nodiscard]] EntityType GetType() const override;

	[[nodiscard]] bool Update(ID3D11DeviceContext *context, Time &time, const Input &input) override;
	[[nodiscard]] bool BindBuffers(ID3D11DeviceContext *context) const override;
	[[nodiscard]] bool Render(CameraD3D11 *camera) override;

	[[nodiscard]] bool PerformDrawCall(ID3D11DeviceContext *context) const;
};
