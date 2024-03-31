
static const float EPSILON = 0.00005f;
static const float NORMAL_OFFSET = 0.005f;

RWTexture2DArray<unorm float4> TargetUAV : register(u0);

Texture2D PositionGBuffer : register(t0); // w is unused
Texture2D ColorGBuffer : register(t1); // w is specularity
Texture2D NormalGBuffer : register(t2); // w is unused

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

StructuredBuffer<SpotLight> SpotLights : register(t3);
Texture2DArray<float> SpotShadowMaps : register(t4);

struct PointLight
{
	float4x4 vp_matrix;
	float3 position;
	float3 color;
	float falloff;
	float specularity;
};

StructuredBuffer<PointLight> PointLights : register(t5);
Texture2DArray<float> PointShadowMaps : register(t6);


// Generic color-clamping algorithm, not mine but it looks good
float3 ACESFilm(const float3 x)
{
	return clamp((x * (2.51f * x + 0.03f)) / (x * (2.43f * x + 0.59f) + 0.14f), 0.0f, 1.0f);
}


[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	const float specularity = (1.0f / pow(1.001f - ColorGBuffer[DTid.xy].w, 1.5f));
	const float3
		pos = PositionGBuffer[DTid.xy].xyz,
		col = ColorGBuffer[DTid.xy].xyz,
		norm = normalize(NormalGBuffer[DTid.xy].xyz),
		viewDir = normalize(cam_position.xyz - pos);
	
	float3 totalDiffuseLight = float3(0.0f, 0.0f, 0.0f);
	float3 totalSpecularLight = float3(0.0f, 0.0f, 0.0f);


	uint spotLightCount, _;
	SpotLights.GetDimensions(spotLightCount, _);

	// Per-spotlight calculations
	for (uint spotlight_i = 0; spotlight_i < spotLightCount; spotlight_i++)
	{
		// Prerequisite variables
		const SpotLight light = SpotLights[spotlight_i];

		const float3
			toLight = light.position - pos,
			toLightDir = normalize(toLight),
			halfwayDir = normalize(toLightDir + viewDir);
		
		const float
			inverseLightDistSqr = 1.0f / (1.0f + (pow(toLight.x * light.falloff, 2) + pow(toLight.y * light.falloff, 2) + pow(toLight.z * light.falloff, 2))),
			maxOffsetAngle = light.angle * 0.5f,
			lightDirOffset = dot(-toLightDir, light.direction),
			offsetAngle = saturate(1.0f - (acos(lightDirOffset) / maxOffsetAngle));
		

		// Calculate Blinn-Phong shading
		float directionScalar = max(dot(norm, toLightDir), 0.0f);
		const float3 diffuseCol = light.color.xyz * directionScalar;
		
		const float specFactor = pow(max(dot(norm, halfwayDir), 0.0f), specularity);
		const float3 specularCol = (
			directionScalar * specularity * smoothstep(0.0f, 1.0f, specFactor) * 
			light.color.xyz / max(max(light.color.x, light.color.y), light.color.z)
		);


		// Calculate shadow projection
		const float4 fragPosLightClip = mul(float4(pos + norm * NORMAL_OFFSET, 1.0f), light.vp_matrix);
		const float3 fragPosLightNDC = fragPosLightClip.xyz / fragPosLightClip.w;

		const bool isInsideFrustum = (
			fragPosLightNDC.x > -1.0f && fragPosLightNDC.x < 1.0f &&
			fragPosLightNDC.y > -1.0f && fragPosLightNDC.y < 1.0f
		);

		const float3 spotUV = float3((fragPosLightNDC.x * 0.5f) + 0.5f, (fragPosLightNDC.y * -0.5f) + 0.5f, spotlight_i);
		const float spotDepth = SpotShadowMaps.SampleLevel(Sampler, spotUV, 0).x;
		const float spotResult = spotDepth + EPSILON > fragPosLightNDC.z ? 1.0f : 0.0f;
		const float shadow = isInsideFrustum * saturate(offsetAngle * spotResult);

		// Apply lighting
		totalDiffuseLight += diffuseCol * shadow * inverseLightDistSqr;
		totalSpecularLight += specularCol * shadow * inverseLightDistSqr;
	}


	uint pointLightCount;
	PointLights.GetDimensions(pointLightCount, _);

	// Per-pointlight calculations
	for (uint pointlight_i = 0; pointlight_i < pointLightCount; pointlight_i++)
	{
		// Prerequisite variables
		const PointLight light = PointLights[pointlight_i];

		const float3
			toLight = light.position - pos,
			toLightDir = normalize(toLight),
			halfwayDir = normalize(toLightDir + viewDir);
		
		const float inverseLightDistSqr = 1.0f / (1.0f + (
			pow(toLight.x * light.falloff, 2) + 
			pow(toLight.y * light.falloff, 2) + 
			pow(toLight.z * light.falloff, 2)
		));
		

		// Calculate Blinn-Phong shading
		float directionScalar = max(dot(norm, toLightDir), 0.0f);
		const float3 diffuseCol = light.color.xyz * directionScalar;
		
		const float specFactor = pow(max(dot(norm, halfwayDir), 0.0f), specularity);
		const float3 specularCol = (
			directionScalar * specularity * smoothstep(0.0f, 1.0f, specFactor) * 
			light.color.xyz / max(max(light.color.x, light.color.y), light.color.z)
		);


		// Calculate shadow projection
		const float4 fragPosLightClip = mul(float4(pos + norm * NORMAL_OFFSET, 1.0f), light.vp_matrix);
		const float3 fragPosLightNDC = fragPosLightClip.xyz / fragPosLightClip.w;

		const bool isInsideFrustum = (
			fragPosLightNDC.x > -1.0f && fragPosLightNDC.x < 1.0f &&
			fragPosLightNDC.y > -1.0f && fragPosLightNDC.y < 1.0f
		);

		const float3 pointUV = float3((fragPosLightNDC.x * 0.5f) + 0.5f, (fragPosLightNDC.y * -0.5f) + 0.5f, pointlight_i);
		const float pointDepth = PointShadowMaps.SampleLevel(Sampler, pointUV, 0).x;
		const float pointResult = pointDepth + EPSILON > fragPosLightNDC.z ? 1.0f : 0.0f;
		const float shadow = isInsideFrustum * saturate(pointResult);

		// Apply lighting
		totalDiffuseLight += diffuseCol * shadow * inverseLightDistSqr;
		totalSpecularLight += specularCol * shadow * inverseLightDistSqr;
	}


	//const float3 result = saturate(col * ((ambient_light.xyz) + totalDiffuseLight) + totalSpecularLight);
	const float3 result = ACESFilm(col * ((ambient_light.xyz) + totalDiffuseLight) + totalSpecularLight);
	TargetUAV[uint3(DTid.xy, 0)] = float4(result, 1.0f);
}