
cbuffer EmitterData : register(b0)
{
	uint particle_count;
	uint particle_rate;
	float lifetime;
	float deltaTime;
};

struct Particle
{
	float3 position;
	float3 velocity;
	float4 color;
	float size;
	float lifetime;
};

RWStructuredBuffer<Particle> Particles : register(u0);


[numthreads(32, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	Particle currParticle = Particles[DTid.x];

	// Update
	currParticle.position += currParticle.velocity * deltaTime;
	currParticle.lifetime += deltaTime;

	if (currParticle.lifetime >= lifetime)
	{
		currParticle.lifetime -= lifetime;
		//currParticle.position = float3(0, 0, 0);
		//currParticle.velocity = float3(0, 0, 0);
	}

	Particles[DTid.x] = currParticle;
}