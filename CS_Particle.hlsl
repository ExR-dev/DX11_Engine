
cbuffer EmitterData : register(b0)
{
	int
		particle_count,
		particle_rate;

	float2
		lifetime_range,
		size_range,
		speed_range;
};

cbuffer Time : register(b1)
{
	float dTime;
};

struct Particle
{
	float3 position;
	float3 velocity;
	float3 color;
	float size, lifetime;
};

RWStructuredBuffer<Particle> Particles : register(u0);


[numthreads(32, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	Particle currParticle = Particles[DTid.x];

	// Update
	currParticle.position += currParticle.velocity * dTime;
	currParticle.lifetime += dTime;

	Particles[DTid.x] = currParticle;
}