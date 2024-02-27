
Texture2D Texture : register(t0);
sampler Sampler : register(s0);


struct PixelShaderInput
{
	float4 position : SV_POSITION;
	float4 world_position : POSITION;
    float3 normal : NORMAL;
	float2 tex_coord : TEXCOORD;
};

struct PixelShaderOutput
{
	float4 position : SV_Target0;
	float4 color : SV_Target1;
	float4 normal : SV_Target2;
};


PixelShaderOutput main(PixelShaderInput input) : SV_TARGET
{	
	PixelShaderOutput output;

	const float3 col = Texture.Sample(Sampler, input.tex_coord).xyz;

	output.position = input.world_position;
	output.color = float4(col, 1.0f);
	output.normal = float4(input.normal, 1.0f);
	return output;
}