
cbuffer WorldMatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix inverseTransposeWorldMatrix;
};

struct Particle
{
	float3 position;
	float3 velocity;
	float4 color;
	float size;
	float lifetime;
};

StructuredBuffer<Particle> Particles : register(t0);


struct ParticleOut
{
	float3 position : POSITION;
	float4 color : COLOR;
	float size : SIZE;
};


ParticleOut main(const uint vertexID : SV_VertexID)
{
	ParticleOut output;
	output.position = mul(float4(Particles[vertexID].position, 1.0f), worldMatrix).xyz;
	output.color = Particles[vertexID].color;
	output.size = Particles[vertexID].size;
	return output;
}