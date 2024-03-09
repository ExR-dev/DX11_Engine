
cbuffer EmitterData : register(b0)
{
	uint
		particle_count,
		particle_rate;
	float
		lifetime, p1;
	//float2
	//	size_range,
	//	speed_range;
};

cbuffer Time : register(b1)
{
	float time, dTime, p2, p3;
};

struct Particle
{
	float3
		position,
		velocity,
		color;
	float
		size,
		lifetime,
		padding;
};

RWStructuredBuffer<Particle> Particles : register(u0);


[numthreads(32, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	Particle currParticle = Particles[DTid.x];

	// Update
	currParticle.position += currParticle.velocity * dTime;
	currParticle.lifetime += dTime;

	if (currParticle.lifetime >= lifetime)
	{
		currParticle.lifetime -= lifetime;
		//currParticle.position = float3(0, 0, 0);
		//currParticle.velocity = float3(0, 0, 0);
	}

	Particles[DTid.x] = currParticle;
}