
cbuffer WorldMatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix inverseTransposeWorldMatrix;
};

cbuffer ViewProjMatrixBuffer : register(b1)
{
    matrix viewProjMatrix;
};


struct VertexShaderInput
{
	float3 position : POSITION;
};

struct VertexShaderOutput
{
	float4 position : POSITION;
};

VertexShaderOutput main(VertexShaderInput input)
{
	VertexShaderOutput output;
	output.position = mul(mul(float4(input.position, 1.0f), worldMatrix), viewProjMatrix);
	return output;
}