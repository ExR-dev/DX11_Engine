
Texture2D Texture : register(t0);
Texture2D NormalMap : register(t1);
Texture2D SpecularMap : register(t2);

sampler Sampler : register(s0);


cbuffer MaterialProperties : register(b2)
{
	int sampleNormal; // Use normal map if greater than zero.
	int sampleSpecular; // Use specular map if greater than zero.
	float padding[2];
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

	const float3 col = Texture.Sample(Sampler, input.tex_coord).xyz;

	const float3 normal = (sampleNormal > 0)
		? mul(NormalMap.Sample(Sampler, input.tex_coord).xyz * 2.0f - float3(1.0f, 1.0f, 1.0f), float3x3(input.tangent, input.bitangent, input.normal))
		: input.normal;

	const float specularity = (sampleSpecular > 0)
		? SpecularMap.Sample(Sampler, input.tex_coord).x
		: 0.0f;

	output.position = float4(input.world_position.xyz, 1.0f);
	output.color = float4(col, specularity);
	output.normal = float4(normal, 0.0f);

	return output;
}