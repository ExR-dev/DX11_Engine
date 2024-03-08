
struct Particle
{
	float3 position;
	float3 velocity;
	float3 color;
	float size;
};

StructuredBuffer<Particle> Particles : register(t0);


float4 main(uint vertexID : SV_VertexID) : POSITION
{
	float4 output = float4(Particles[vertexID].position, Particles[vertexID].size);
	return output;
}