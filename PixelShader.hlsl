
Texture2D Texture : register(t0);
sampler Sampler : register(s0);

cbuffer LightingData : register(b0)
{
	float4 camPosition;
	float4 lightPosition;

	float4 ambient;
	float4 diffuse;
	float4 specular;
};

struct PixelShaderInput
{
	float4 position : SV_POSITION;
	float4 world_position : POSITION;
    float3 normal : NORMAL;
	float2 tex_coord : TEXCOORD;
};

float4 main(PixelShaderInput input) : SV_TARGET
{	
	// Prerequisite variables
	const float3 col = Texture.Sample(Sampler, input.tex_coord).xyz;
	const float3 toLight = lightPosition.xyz - input.world_position.xyz;
	const float lightDistSqr = pow(toLight.x, 2) + pow(toLight.y, 2) + pow(toLight.z, 2);

	const float3 lightDir = normalize(toLight);
	const float3 viewDir = normalize(camPosition.xyz - input.world_position.xyz);
	const float3 halfwayDir = normalize(lightDir + viewDir);
	
	// Lighting types
	float3 ambCol = col * ambient.xyz * ambient.w;
	
	float3 diffCol = col * diffuse.xyz * diffuse.w * max(dot(input.normal, lightDir), 0);
	
	float specFactor = pow(saturate(dot(input.normal, halfwayDir)), specular.w);
	float3 specCol = specular.xyz * specFactor;
	
	// Apply lighting
	float3 lighting = ambCol + (diffCol + specCol) / lightDistSqr;
	return float4(saturate(lighting), 1.0f);
}