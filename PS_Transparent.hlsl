
Texture2D Texture : register(t0);
sampler Sampler : register(s0);

cbuffer GlobalLight : register(b0)
{
	float4 ambient_light;
};

cbuffer CameraData : register(b1)
{
	float4 cam_position;
};

struct SpotLight
{
	float4x4 vp_matrix;
	float3 color;
	float3 direction;
	float angle;
	float falloff;
	float specularity;
	float3 light_position;
};

StructuredBuffer<SpotLight> SpotLights : register(t1);
Texture2DArray<float> ShadowMaps : register(t2);


// Generic color-clamping algorithm, not mine but it looks good
float3 ACESFilm(const float3 x)
{
	return clamp((x * (2.51f * x + 0.03f)) / (x * (2.43f * x + 0.59f) + 0.14f), 0.0f, 1.0f);
}


struct PixelShaderInput
{
	float4 position : SV_POSITION;
	float4 world_position : POSITION;
    float3 normal : NORMAL;
	float2 tex_coord : TEXCOORD;
};

float4 main(PixelShaderInput input) : SV_TARGET
{	
	float4 col = Texture.Sample(Sampler, input.tex_coord);

	const float3 viewDir = normalize(cam_position.xyz - input.world_position.xyz);

	uint lightCount, _;
	SpotLights.GetDimensions(lightCount, _);
	
	float3 totalDiffuseLight = float3(0.0f, 0.0f, 0.0f);
	float3 totalSpecularLight = float3(0.0f, 0.0f, 0.0f);

	// Per-light calculations
	for (uint light_i = 0; light_i < lightCount; light_i++)
	{
		// Prerequisite variables
		const SpotLight light = SpotLights[light_i];

		const float3
			toLight = light.light_position - input.world_position.xyz,
			toLightDir = normalize(toLight),
			halfwayDir = normalize(toLightDir + viewDir);
		
		const float
			inverseLightDistSqr = 1.0f / (1.0f + (pow(toLight.x * light.falloff, 2) + pow(toLight.y * light.falloff, 2) + pow(toLight.z * light.falloff, 2))),
			maxOffsetAngle = light.angle * 0.5f,
			lightDirOffset = dot(-toLightDir, light.direction),
			offsetAngle = saturate(1.0f - (acos(lightDirOffset) / maxOffsetAngle));
		

		// Calculate Blinn-Phong shading
		const float3 diffuseCol = light.color.xyz * max(abs(dot(input.normal, toLightDir)), 0.0f);
		
		const float specFactor = pow(saturate(abs(dot(input.normal, halfwayDir))), light.specularity);
		const float3 specularCol = float3(1.0f, 1.0f, 1.0f) * specFactor;


		// Calculate shadow projection
		const float4 fragPosLightClip = mul(float4(input.world_position.xyz, 1.0f), light.vp_matrix);
		const float3 fragPosLightNDC = fragPosLightClip.xyz / fragPosLightClip.w;

		const float3 smUV = float3((fragPosLightNDC.x * 0.5f) + 0.5f, (fragPosLightNDC.y * -0.5f) + 0.5f, light_i);
		const float smDepth = ShadowMaps.SampleLevel(Sampler, smUV, 0).x;
		const float smResult = smDepth > fragPosLightNDC.z ? 1.0f : 0.0f;

		const float shadow = saturate(offsetAngle * smResult);

		// Apply lighting
		totalDiffuseLight += diffuseCol * shadow * inverseLightDistSqr;
		totalSpecularLight += specularCol * shadow * inverseLightDistSqr;
	}

	const float3 result = ACESFilm(col.xyz * ((ambient_light.xyz) + totalDiffuseLight + totalSpecularLight));
	return float4(result, min(col.w + max(totalSpecularLight.x, max(totalSpecularLight.y, totalSpecularLight.z)), 1.0f));
}



