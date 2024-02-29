
static const float PI = 3.14159265f;
static const float EPSILON = 0.0005f;


RWTexture2D<unorm float4> BackBufferUAV : register(u0);

Texture2D<float4> PositionGBuffer : register(t0);
Texture2D<float4> ColorGBuffer : register(t1);
Texture2D<float4> NormalGBuffer : register(t2);


cbuffer GlobalLight : register(b0)
{
	float4 ambient_light;
};

struct SpotLight
{
	float4x4 vp_matrix;
	float3 color;
	float3 direction;
	float angle;
	float3 light_position;
};

StructuredBuffer<SpotLight> SpotLights : register(t3);

Texture2DArray<float> ShadowMaps : register(t4);

sampler ShadowMapSampler : register(s0);


cbuffer CameraData : register(b1)
{
	float4 cam_position;
};

float3 ACESFilm(const float3 x)
{
	return clamp((x * (2.51f * x + 0.03f)) / (x * (2.43f * x + 0.59f) + 0.14f), 0.0f, 1.0f);
}

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	const float3 pos = PositionGBuffer[DTid.xy].xyz;
	const float3 col = ColorGBuffer[DTid.xy].xyz;
	const float3 norm = normalize(NormalGBuffer[DTid.xy].xyz);
	
	float3 totalDiffuseLight = float3(0.0f, 0.0f, 0.0f);
	float3 totalSpecularLight = float3(0.0f, 0.0f, 0.0f);

	uint lightCount, lightSize;
	SpotLights.GetDimensions(lightCount, lightSize);

	for (uint lightIndex = 0; lightIndex < lightCount; lightIndex++)
	{
		// Prerequisite variables
		const SpotLight light = SpotLights[lightIndex];

		const float3 toLight = light.light_position - pos;
		const float3 toLightDir = normalize(toLight);
		const float3 viewDir = normalize(cam_position.xyz - pos);
		const float3 halfwayDir = normalize(toLightDir + viewDir);

		const float inverseLightDistSqr = 1.0f / (pow(toLight.x, 2) + pow(toLight.y, 2) + pow(toLight.z, 2));
		const float maxOffsetAngle = light.angle / 2.0f;

		const float lightDirOffset = dot(-toLightDir, light.direction);
		float offsetAngle = saturate(1.0f - (acos(lightDirOffset) / maxOffsetAngle));

		// Lighting types
		const float3 diffCol = light.color.xyz * max(dot(norm, toLightDir), 0); // * inverseLightDistSqr;
		
		const float specFactor = pow(saturate(dot(norm, halfwayDir)), 2);
		const float3 specCol = float3(1, 1, 1) * smoothstep(0, 1, specFactor); // * inverseLightDistSqr);
		
		float4 fragPosLightClip = mul(float4(pos, 1.0f), light.vp_matrix);
		float3 fragPosLightNDC = fragPosLightClip.xyz / fragPosLightClip.w;

		const float3 shadowMapUV = float3((fragPosLightNDC.x * 0.5f) + 0.5f, (fragPosLightNDC.y * -0.5f) + 0.5f, lightIndex);

		const float shadowMapDepth = ShadowMaps.SampleLevel(ShadowMapSampler, shadowMapUV, 0).x;
		const float shadowFactor = fragPosLightNDC.z <= shadowMapDepth + EPSILON ? 1.0f : 0.0f;

		// Apply lighting
		totalDiffuseLight += diffCol * offsetAngle * shadowFactor * sqrt(saturate(1.0f - fragPosLightNDC.z));
		totalSpecularLight += specCol * offsetAngle * shadowFactor * sqrt(saturate(1.0f - fragPosLightNDC.z));
	}

	const float3 result = col * ACESFilm((ambient_light.xyz * ambient_light.w) + totalDiffuseLight) + totalSpecularLight;
	BackBufferUAV[DTid.xy] = float4(saturate(result), 1.0f);
}