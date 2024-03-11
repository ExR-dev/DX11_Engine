
Texture2D Texture : register(t0);
Texture2D NormalMap : register(t1);
Texture2D SpecularMap : register(t2);

sampler Sampler : register(s0);

cbuffer WorldMatrixBuffer : register(b0) // TODO: Move transformations back to vertex shader and remove this buffer
{
	matrix worldMatrix;
	matrix inverseTransposeWorldMatrix;
};

struct PixelShaderInput
{
	float4 position			: SV_POSITION;
	float4 world_position	: POSITION;
    float3 normal			: NORMAL;
    float3 tangent			: TANGENT;
    float3 bitangent		: BITANGENT;
	float2 tex_coord		: TEXCOORD;
};

struct PixelShaderOutput
{
	float4 position : SV_Target0;
	float4 color	: SV_Target1;
	float4 normal	: SV_Target2;
};


PixelShaderOutput main(PixelShaderInput input)
{
	PixelShaderOutput output;
	// TODO: Tangent bug has to do with the normal map, not the tangent space.
	// TODO: Constant normal works as expected but texture with same value does not, find out why.

	const float3 col = Texture.Sample(Sampler, input.tex_coord).xyz;
	const float3 sampleNormal = NormalMap.Sample(Sampler, input.tex_coord).xyz * 2.0f - float3(1.0f, 1.0f, 1.0f); 
	//const float3 sampleNormal = float3(0.0f, 0.0f, 1.0f);
	const float specularity = SpecularMap.Sample(Sampler, input.tex_coord).x;

	//const float3x3 tbn = float3x3(input.tangent, input.bitangent, input.normal);
	//const float3 wsNormal = mul(float4(mul(sampleNormal, tbn), 0.0f), inverseTransposeWorldMatrix).xyz;
	//const float3 wsNormal = mul(float4(sampleNormal, 0.0f), inverseTransposeWorldMatrix).xyz;
	//const float3 wsNormal = mul(sampleNormal, tbn);
	const float3 wsNormal = mul(float4(input.normal, 0.0f), inverseTransposeWorldMatrix).xyz;

	output.position = float4(input.world_position.xyz, 1.0f);
	output.color = float4(col, specularity);
	//output.normal = float4(wsNormal * 0.5f + float3(0.5f, 0.5f, 0.5f), 0.0f);
	output.normal = float4(wsNormal, 0.0f);
	return output;
}