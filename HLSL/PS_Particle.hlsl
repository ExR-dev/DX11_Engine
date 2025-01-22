
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
	float3 position;
	float3 direction;
	float3 color;
	float angle;
	float falloff;
	float specularity;
};

StructuredBuffer<SpotLight> SpotLights : register(t4);
Texture2DArray<float> SpotShadowMaps : register(t5);

struct PointLight
{
	float4x4 vp_matrix;
	float3 position;
	float3 color;
	float falloff;
	float specularity;
};

StructuredBuffer<PointLight> PointLights : register(t6);
Texture2DArray<float> PointShadowMaps : register(t7);


// Generic color-clamping algorithm, not mine but it looks good
float3 ACESFilm(const float3 x)
{
	return clamp((x * (2.51f * x + 0.03f)) / (x * (2.43f * x + 0.59f) + 0.14f), 0.0f, 1.0f);
}


struct PixelShaderInput
{
	float4 position			: SV_POSITION;
	float4 world_position	: POSITION;
	float3 normal			: NORMAL;
	float4 color			: COLOR;
	float2 tex_coord		: TEXCOORD;
};

float4 main(PixelShaderInput input) : SV_TARGET
{	
	const float4 col = Texture.Sample(Sampler, input.tex_coord) * input.color;
	
	float3 totalDiffuseLight = float3(0.0f, 0.0f, 0.0f);


	uint spotLightCount, _;
	SpotLights.GetDimensions(spotLightCount, _);

	// Per-spotlight calculations
	for (uint spotlight_i = 0; spotlight_i < spotLightCount; spotlight_i++)
	{
		// Prerequisite variables
		const SpotLight light = SpotLights[spotlight_i];

		const float3
			toLight = light.position - input.world_position.xyz,
			toLightDir = normalize(toLight);

		const float
			inverseLightDistSqr = 1.0f / (1.0f + (pow(toLight.x * light.falloff, 2) + pow(toLight.y * light.falloff, 2) + pow(toLight.z * light.falloff, 2))),
			maxOffsetAngle = light.angle * 0.5f,
			lightDirOffset = dot(-toLightDir, light.direction),
			offsetAngle = saturate(1.0f - acos(lightDirOffset) / maxOffsetAngle);


		// Calculate shadow projection
		const float4 fragPosLightClip = mul(float4(input.world_position.xyz, 1.0f), light.vp_matrix);
		const float3 fragPosLightNDC = fragPosLightClip.xyz / fragPosLightClip.w;

		const bool isInsideFrustum = (
			fragPosLightNDC.x > -1.0f && fragPosLightNDC.x < 1.0f &&
			fragPosLightNDC.y > -1.0f && fragPosLightNDC.y < 1.0f
		);

		const float3 spotUV = float3((fragPosLightNDC.x * 0.5f) + 0.5f, (fragPosLightNDC.y * -0.5f) + 0.5f, spotlight_i);
		const float spotDepth = SpotShadowMaps.SampleLevel(Sampler, spotUV, 0).x;
		const float spotResult = spotDepth < fragPosLightNDC.z ? 1.0f : 0.0f;

		const float shadow = isInsideFrustum * saturate(offsetAngle * spotResult);


		// Apply lighting
		totalDiffuseLight += light.color.xyz * shadow * inverseLightDistSqr;
	}


	uint pointLightCount;
	PointLights.GetDimensions(pointLightCount, _);

	// Per-pointlight calculations
	for (uint pointlight_i = 0; pointlight_i < pointLightCount; pointlight_i++)
	{
		// Prerequisite variables
		const PointLight light = PointLights[pointlight_i];

		const float3
			toLight = light.position - input.world_position.xyz;

		const float inverseLightDistSqr = 1.0f / (1.0f + (
			pow(toLight.x * light.falloff, 2) + 
			pow(toLight.y * light.falloff, 2) + 
			pow(toLight.z * light.falloff, 2)
		));


		// Calculate shadow projection
		const float4 fragPosLightClip = mul(float4(input.world_position.xyz, 1.0f), light.vp_matrix);
		const float3 fragPosLightNDC = fragPosLightClip.xyz / fragPosLightClip.w;
		
		const bool isInsideFrustum = (
			fragPosLightNDC.x > -1.0f && fragPosLightNDC.x < 1.0f &&
			fragPosLightNDC.y > -1.0f && fragPosLightNDC.y < 1.0f
		);

		const float3 pointUV = float3((fragPosLightNDC.x * 0.5f) + 0.5f, (fragPosLightNDC.y * -0.5f) + 0.5f, pointlight_i);
		const float pointDepth = PointShadowMaps.SampleLevel(Sampler, pointUV, 0).x;
		const float pointResult = pointDepth > fragPosLightNDC.z ? 1.0f : 0.0f;
		const float shadow = isInsideFrustum * saturate(pointResult);


		// Apply lighting
		totalDiffuseLight += light.color.xyz * shadow * inverseLightDistSqr;
	}

	const float3 result = saturate(col.xyz * (ambient_light.xyz + totalDiffuseLight));
	return float4(result, col.w);
}