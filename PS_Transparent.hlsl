
Texture2D Texture : register(t0);
sampler Sampler : register(s0);


struct PixelShaderInput
{
	float4 position : SV_POSITION;
	float4 world_position : POSITION;
    float3 normal : NORMAL;
	float2 tex_coord : TEXCOORD;
};


float4 main(PixelShaderInput input) : SV_TARGET
{	
	return Texture.Sample(Sampler, input.tex_coord);
}