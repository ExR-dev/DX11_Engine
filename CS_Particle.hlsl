
struct Particle
{
	float3 position;
	float3 velocity;
	float3 color;
	float size;
};

RWStructuredBuffer<Particle> Particles : register(u0);


[numthreads(32, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	Particle currParticle = Particles[DTid.x];

	// Update

	Particles[DTid.x] = currParticle;
}