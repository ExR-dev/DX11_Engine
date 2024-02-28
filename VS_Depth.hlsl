
cbuffer WorldMatrixBuffer : register(b0)
{
    matrix worldMatrix;
};

cbuffer ViewProjMatrixBuffer : register(b1)
{
    matrix viewProjMatrix;
};


struct VertexShaderInput
{
	float3 position : POSITION;
    float3 normal : NORMAL;
	float2 tex_coord : TEXCOORD;
};

float4 main(VertexShaderInput input) : SV_POSITION
{
	float4 output = mul(mul(float4(input.position, 1.0f), worldMatrix), viewProjMatrix);
	return output;
}