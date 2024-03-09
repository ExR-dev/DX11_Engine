
cbuffer WorldMatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix inverseTransposeWorldMatrix;
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

StructuredBuffer<Particle> Particles : register(t0);


//truct ParticleOut
//
//	float3 position : POSITION;
//	float3 color : COLOR;
//	float size : SIZE;
//;


float4 main(const uint vertexID : SV_VertexID) : POSITION
{
	//ParticleOut output;
	//output.position = Particles[vertexID].position;
	//output.color = Particles[vertexID].color;
	//output.size = Particles[vertexID].size;
	//return output;
	return float4(mul(float4(Particles[vertexID].position, 1.0f), worldMatrix).xyz, Particles[vertexID].size);
}