#pragma once

#include "Entity.h"


struct EmitterData
{
	UINT maxParticles;

	float (*lifetimeFunc)(float, float);
	float minLifetime, maxLifetime;

	float (*sizeFunc)(float, float);
	float minSize, maxSize;

	float (*speedFunc)(float, float);
	float minSpeed, maxSpeed;
};

struct Particle
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 velocity;
	float age;
};


class Emitter final : Entity
{
private:
	EmitterData _settings = { };
	std::vector<Particle *> _liveParticles, _deadParticles;

	StructuredBufferD3D11 _particleBuffer;

public:
	explicit Emitter(UINT id, const DirectX::BoundingBox &bounds);

	[[nodiscard]] bool Initialize(ID3D11Device *device, const EmitterData *settings = nullptr);

	[[nodiscard]] EntityType GetType() const override;

	[[nodiscard]] bool Update(ID3D11DeviceContext *context, Time &time, const Input &input) override;
	[[nodiscard]] bool BindBuffers(ID3D11DeviceContext *context) const override;
	[[nodiscard]] bool Render(CameraD3D11 *camera) override;
};
