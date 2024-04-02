
Texture2D Texture				: register(t0);
Texture2D NormalMap				: register(t1);
Texture2D SpecularMap			: register(t2);
Texture2D ReflectionMap			: register(t3);
Texture2D AmbientMap			: register(t4);

sampler Sampler					: register(s0);


cbuffer MaterialProperties : register(b2)
{
	int sampleNormal; // Use normal map if greater than zero.
	int sampleSpecular; // Use specular map if greater than zero.
	int sampleReflection; // Use reflection map & environment cubemap if greater than zero.
	int sampleAmbient; // Use ambient map if greater than zero.
};


struct PixelShaderInput
{
	float4 position			: SV_POSITION;
	float4 world_position	: POSITION;
    float3 normal			: NORMAL;
    float3 tangent			: TANGENT;
	float2 tex_coord		: TEXCOORD;
};

struct PixelShaderOutput
{
	float4 position : SV_Target0; // w is specular r
	float4 normal	: SV_Target1; // w is specular g
	float4 ambient	: SV_Target2; // w is specular b
	float4 diffuse	: SV_Target3; // w is reflectivity
};

PixelShaderOutput main(PixelShaderInput input)
{
	PixelShaderOutput output;

	const float3 diffuse = Texture.Sample(Sampler, input.tex_coord).xyz;
	
	const float3 bitangent = normalize(cross(input.normal, input.tangent));
	const float3 normal = (sampleNormal > 0)
		? mul(NormalMap.Sample(Sampler, input.tex_coord).xyz * 2.0f - float3(1.0f, 1.0f, 1.0f), float3x3(input.tangent, bitangent, input.normal))
		: input.normal;

	const float4 specularity = (sampleSpecular > 0)
		? SpecularMap.Sample(Sampler, input.tex_coord)
		: float4(0.0f, 0.0f, 0.0f, 0.0f);

	const float3 ambient = (sampleAmbient > 0)
		? AmbientMap.Sample(Sampler, input.tex_coord).xyz
		: float3(0.0f, 0.0f, 0.0f);

	const float reflectivity = (sampleReflection > 0)
		? ReflectionMap.Sample(Sampler, input.tex_coord).x
		: 0.f;

	output.position = float4(input.world_position.xyz, specularity.x);
	output.normal	= float4(normal, specularity.y);
	output.ambient	= float4(ambient, specularity.z);
	output.diffuse = float4(diffuse, (float)((uint) (specularity.w * 512.0f)) + reflectivity * 0.99f);

	return output;
}