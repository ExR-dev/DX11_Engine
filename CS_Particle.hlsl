
const float PI = 3.14159265359f;

cbuffer EmitterData : register(b0)
{
	uint particle_count;
	uint particle_rate;
	float lifetime;
	float currTime;
};

struct Particle
{
	float4 position;
	float4 velocity;
	float4 color;
	float4 startPos;
	float4 endPos;
	//float size;
	//float particle_lifetime;
};

RWStructuredBuffer<Particle> Particles : register(u0);


[numthreads(32, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	Particle currParticle = Particles[DTid.x];

	int _;
	currParticle.position.x = lerp(currParticle.startPos.x, currParticle.endPos.x, smoothstep(0.0f, 1.0f, abs(modf(currTime / 8.785f + 0.17477f + currParticle.velocity.w, _) * 2.0f - 1.0f)));
	currParticle.position.y = lerp(currParticle.startPos.y, currParticle.endPos.y, smoothstep(0.0f, 1.0f, abs(modf(currTime / 9.631f + 0.46446f + currParticle.velocity.w, _) * 2.0f - 1.0f)));
	currParticle.position.z = lerp(currParticle.startPos.z, currParticle.endPos.z, smoothstep(0.0f, 1.0f, abs(modf(currTime / 10.29f + 0.74168f + currParticle.velocity.w, _) * 2.0f - 1.0f)));

	//currParticle.position += 0.02f * currParticle.velocity * sin(PI * 2.0f * currParticle.lifetime / lifetime);
	//currParticle.position.xyz = lerp(currParticle.startPos.xyz, currParticle.endPos.xyz, sin(PI * 2.0f * modf(currTime, _)));
	//currParticle.position.x = lerp(currParticle.startPos.x, currParticle.endPos.x, abs((2.0f * currParticle.lifetime / lifetime) - 1.0f));
	//currParticle.position.y = lerp(currParticle.startPos.y, currParticle.endPos.y, abs((2.0f * currParticle.lifetime / lifetime) - 1.0f));
	//currParticle.position.z = lerp(currParticle.startPos.z, currParticle.endPos.z, abs((2.0f * currParticle.lifetime / lifetime) - 1.0f));

	//currParticle.particle_lifetime += deltaTime;
	/*if (currParticle.lifetime >= lifetime)
	{
		currParticle.lifetime -= lifetime;
		currParticle.position = currParticle.startPos;
		//currParticle.position -= currParticle.velocity * lifetime;
		//currParticle.position = float3(0, 0, 0);
		//currParticle.velocity = float3(0, 0, 0);
	}*/

	Particles[DTid.x] = currParticle;
}