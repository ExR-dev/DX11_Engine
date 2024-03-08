
cbuffer CameraData : register(b0)
{
	float4 cam_position;
};

struct GeometryShaderOutput
{
	float4 position : SV_POSITION;
	float4 world_position : POSITION;
	float3 normal : NORMAL;
	float2 tex_coord : TEXCOORD;
};

[maxvertexcount(6)]
void main(point float3 input[1] : POSITION, 
	inout TriangleStream<GeometryShaderOutput> output)
{

}