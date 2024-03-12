
static const float EPSILON = 0.00001f;
static const float NORMAL_OFFSET = 0.005f;

RWTexture2D<unorm float4> BackBufferUAV : register(u0);

Texture2D<float4> PositionGBuffer : register(t0); // w is unused
Texture2D<float4> ColorGBuffer : register(t1); // w is specularity
Texture2D<float4> NormalGBuffer : register(t2); // w is unused


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
	float falloff;
	float specularity;
	float3 light_position;
};

StructuredBuffer<SpotLight> SpotLights : register(t3);

Texture2DArray<float> ShadowMaps : register(t4);

sampler ShadowMapSampler : register(s0);


cbuffer CameraData : register(b1)
{
	float4 cam_position;
};

// Generic color-clamping algorithm, not mine but it looks good
float3 ACESFilm(const float3 x)
{
	return clamp((x * (2.51f * x + 0.03f)) / (x * (2.43f * x + 0.59f) + 0.14f), 0.0f, 1.0f);
}


[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	const float specularity = (1.0f / pow(1.0f + EPSILON - ColorGBuffer[DTid.xy].w, 1.5f));
	const float3
		pos = PositionGBuffer[DTid.xy].xyz,
		col = ColorGBuffer[DTid.xy].xyz,
		norm = normalize(NormalGBuffer[DTid.xy].xyz),
		viewDir = normalize(cam_position.xyz - pos);

	uint lightCount, smWidth, smHeight, _;
	SpotLights.GetDimensions(lightCount, _);
	ShadowMaps.GetDimensions(0, smWidth, smHeight, _, _);

	const float
		smDX = 1.0f / (float)smWidth,
		smDY = 1.0f / (float)smHeight;
	
	float3 totalDiffuseLight = float3(0.0f, 0.0f, 0.0f);
	float3 totalSpecularLight = float3(0.0f, 0.0f, 0.0f);

	// Per-light calculations
	for (uint light_i = 0; light_i < lightCount; light_i++)
	{
		// Prerequisite variables
		const SpotLight light = SpotLights[light_i];

		const float3
			toLight = light.light_position - pos,
			toLightDir = normalize(toLight),
			halfwayDir = normalize(toLightDir + viewDir);
		
		const float
			inverseLightDistSqr = 1.0f / (1.0f + (pow(toLight.x * light.falloff, 2) + pow(toLight.y * light.falloff, 2) + pow(toLight.z * light.falloff, 2))),
			maxOffsetAngle = light.angle * 0.5f,
			lightDirOffset = dot(-toLightDir, light.direction),
			offsetAngle = saturate(1.0f - (acos(lightDirOffset) / maxOffsetAngle));
		

		// Calculate Blinn-Phong shading
		float directionScalar = dot(norm, toLightDir);
		const float3 diffuseCol = light.color.xyz * max(directionScalar, 0.0f);
		
		const float specFactor = pow(saturate(dot(norm, halfwayDir)), specularity);
		//const float3 specularCol = light.specularity * smoothstep(0.0f, 1.0f, specFactor) * float3(1.0f, 1.0f, 1.0f);
		const float3 specularCol = (directionScalar > 0.0f ? 1.0f : 0.0f) * specularity * smoothstep(0.0f, 1.0f, specFactor) * float3(1.0f, 1.0f, 1.0f);


		// Calculate shadow projection
		const float4 fragPosLightClip = mul(float4(pos + norm * NORMAL_OFFSET, 1.0f), light.vp_matrix);
		const float3 fragPosLightNDC = fragPosLightClip.xyz / fragPosLightClip.w;

		const float3
			smUV00 = float3((fragPosLightNDC.x * 0.5f) + 0.5f, (fragPosLightNDC.y * -0.5f) + 0.5f, light_i),
			smUV01 = smUV00 + float3(0.0f, smDY, 0.0f),
			smUV10 = smUV00 + float3(smDX, 0.0f, 0.0f),
			smUV11 = smUV00 + float3(smDX, smDY, 0.0f);

		const float
			smDepth00 = ShadowMaps.SampleLevel(ShadowMapSampler, smUV00, 0).x,
			smDepth01 = ShadowMaps.SampleLevel(ShadowMapSampler, smUV01, 0).x,
			smDepth10 = ShadowMaps.SampleLevel(ShadowMapSampler, smUV10, 0).x,
			smDepth11 = ShadowMaps.SampleLevel(ShadowMapSampler, smUV11, 0).x;

		const float
			smResult00 = smDepth00 + EPSILON > fragPosLightNDC.z ? 1.0f : 0.0f,
			smResult01 = smDepth01 + EPSILON > fragPosLightNDC.z ? 1.0f : 0.0f,
			smResult10 = smDepth10 + EPSILON > fragPosLightNDC.z ? 1.0f : 0.0f,
			smResult11 = smDepth11 + EPSILON > fragPosLightNDC.z ? 1.0f : 0.0f;

		const float2
			texelPos = smUV00.xy * (float)smWidth,
			fracTex = frac(texelPos);
		
		const float shadow = saturate(
			offsetAngle * lerp(
				lerp(smResult00, smResult10, fracTex.x),
				lerp(smResult01, smResult11, fracTex.x),
				fracTex.y)
		);
		//const float shadow = saturate(offsetAngle * smResult00);


		// Apply lighting
		totalDiffuseLight += diffuseCol * shadow * inverseLightDistSqr;
		totalSpecularLight += specularCol * shadow * inverseLightDistSqr;
	}

	const float3 result = ACESFilm(col * ((ambient_light.xyz) + totalDiffuseLight) + totalSpecularLight);
	BackBufferUAV[DTid.xy] = float4(result, 1.0f);

	//BackBufferUAV[DTid.xy] = float4(norm * 0.5f + float3(0.5f, 0.5f, 0.5f), 1.0f);
	//BackBufferUAV[DTid.xy] = float4(abs(norm), 1.0f);
	//BackBufferUAV[DTid.xy] = float4(norm, 1.0f);
}