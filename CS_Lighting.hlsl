
static const float EPSILON = 0.00005f;
static const float NORMAL_OFFSET = 0.005f;

RWTexture2D<unorm float4> BackBufferUAV : register(u0);

Texture2D PositionGBuffer : register(t0); // w is unused
Texture2D ColorGBuffer : register(t1); // w is specularity
Texture2D NormalGBuffer : register(t2); // w is reflectivity

sampler Sampler : register(s0);


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

struct PointLight
{
	float3 color;
	float3 light_position;
	float falloff;
	float specularity;
	float2 nearFarPlanes;
	float padding[2];
};

StructuredBuffer<PointLight> PointLights : register(t5);
TextureCubeArray<float> ShadowCubemaps : register(t6);

TextureCube EnvironmentCubemap	: register(t7);


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
	const float specularity = (1.0f / pow(1.001f - ColorGBuffer[DTid.xy].w, 1.5f));
	const float reflectivity = NormalGBuffer[DTid.xy].w;
	const float3
		pos = PositionGBuffer[DTid.xy].xyz,
		col = ColorGBuffer[DTid.xy].xyz,
		norm = normalize(NormalGBuffer[DTid.xy].xyz),
		viewDir = normalize(cam_position.xyz - pos);

	const float3 reflection = (reflectivity > 0.01f)
		? EnvironmentCubemap.SampleLevel(Sampler, reflect(viewDir, norm) * float3(-1, 1, 1), 0).xyz
		: float3(0,0,0);

	float3 totalDiffuseLight = float3(0.0f, 0.0f, 0.0f);
	float3 totalSpecularLight = float3(0.0f, 0.0f, 0.0f);

	uint spotlightCount, spotWidth, spotHeight, _;
	SpotLights.GetDimensions(spotlightCount, _);
	ShadowMaps.GetDimensions(0, spotWidth, spotHeight, _, _);

	const float
		spotDX = 1.0f / (float)spotWidth,
		spotDY = 1.0f / (float)spotHeight;

	// Per-spotlight calculations
	for (uint light_i = 0; light_i < spotlightCount; light_i++)
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
		float directionScalar = max(dot(norm, toLightDir), 0.0f);
		const float3 diffuseCol = light.color.xyz * directionScalar;
		
		const float specFactor = pow(max(dot(norm, halfwayDir), 0.0f), specularity);
		const float3 specularCol = directionScalar * specularity * smoothstep(0.0f, 1.0f, specFactor) * light.color.xyz / max(max(light.color.x, light.color.y), light.color.z);


		// Calculate shadow projection
		const float4 fragPosLightClip = mul(float4(pos + norm * NORMAL_OFFSET, 1.0f), light.vp_matrix);
		const float3 fragPosLightNDC = fragPosLightClip.xyz / fragPosLightClip.w;

		/*
		const float3 spotUV = float3((fragPosLightNDC.x * 0.5f) + 0.5f, (fragPosLightNDC.y * -0.5f) + 0.5f, light_i);
		const float spotDepth = ShadowMaps.SampleLevel(Sampler, spotUV, 0).x;
		const float spotResult = spotDepth + EPSILON > fragPosLightNDC.z ? 1.0f : 0.0f;
		const float shadow = saturate(offsetAngle * spotResult);
		*/

		const float3
			spotUV00 = float3((fragPosLightNDC.x * 0.5f) + 0.5f, (fragPosLightNDC.y * -0.5f) + 0.5f, light_i),
			spotUV01 = spotUV00 + float3(0.0f, spotDY, 0.0f),
			spotUV10 = spotUV00 + float3(spotDX, 0.0f, 0.0f),
			spotUV11 = spotUV00 + float3(spotDX, spotDY, 0.0f);

		const float
			spotDepth00 = ShadowMaps.SampleLevel(Sampler, spotUV00, 0).x,
			spotDepth01 = ShadowMaps.SampleLevel(Sampler, spotUV01, 0).x,
			spotDepth10 = ShadowMaps.SampleLevel(Sampler, spotUV10, 0).x,
			spotDepth11 = ShadowMaps.SampleLevel(Sampler, spotUV11, 0).x;

		const float
			spotResult00 = spotDepth00 + EPSILON > fragPosLightNDC.z ? 1.0f : 0.0f,
			spotResult01 = spotDepth01 + EPSILON > fragPosLightNDC.z ? 1.0f : 0.0f,
			spotResult10 = spotDepth10 + EPSILON > fragPosLightNDC.z ? 1.0f : 0.0f,
			spotResult11 = spotDepth11 + EPSILON > fragPosLightNDC.z ? 1.0f : 0.0f;

		const float2
			texelPos = spotUV00.xy * (float)spotWidth,
			fracTex = frac(texelPos);
		
		const float shadow = saturate(
			offsetAngle * lerp(
				lerp(spotResult00, spotResult10, fracTex.x),
				lerp(spotResult01, spotResult11, fracTex.x),
				fracTex.y)
		);


		// Apply lighting
		totalDiffuseLight += diffuseCol * shadow * inverseLightDistSqr;
		totalSpecularLight += specularCol * shadow * inverseLightDistSqr;
	}

	uint pointlightCount, pointWidth, pointHeight;
	PointLights.GetDimensions(pointlightCount, _);
	ShadowCubemaps.GetDimensions(0, pointWidth, pointHeight, _, _);

	const float
		pointDX = 1.0f / (float)pointWidth,
		pointDY = 1.0f / (float)pointHeight;

	// Per-pointlight calculations
	for (uint light_i = 0; light_i < pointlightCount; light_i++)
	{
		// Prerequisite variables
		const PointLight light = PointLights[light_i];

		const float3
			toLight = light.light_position - pos,
			toLightDir = normalize(toLight),
			halfwayDir = normalize(toLightDir + viewDir);
		
		const float
			inverseLightDistSqr = 1.0f / (1.0f + (pow(toLight.x * light.falloff, 2) + pow(toLight.y * light.falloff, 2) + pow(toLight.z * light.falloff, 2)));
		

		// Calculate Blinn-Phong shading
		float directionScalar = max(dot(norm, toLightDir), 0.0f);
		const float3 diffuseCol = light.color.xyz * directionScalar;
		
		const float specFactor = pow(max(dot(norm, halfwayDir), 0.0f), specularity);
		const float3 specularCol = directionScalar * specularity * smoothstep(0.0f, 1.0f, specFactor) * light.color.xyz / max(max(light.color.x, light.color.y), light.color.z);


		// Calculate shadow projection
		//const float3 fragPosLightClip = saturate((abs((pos + norm * NORMAL_OFFSET) - light.light_position) - light.nearFarPlanes.x) / light.nearFarPlanes.y);
		const float3 fragPosLightClip = saturate((abs((pos + norm * NORMAL_OFFSET) - light.light_position)));

		const float3 dimDot = abs(float3(
			dot(-toLightDir, float3(1, 0, 0)),
			dot(-toLightDir, float3(0, 1, 0)),
			dot(-toLightDir, float3(0, 0, 1))
		));

		float fragDepth = (dimDot.x > dimDot.y) ? 
			((dimDot.x > dimDot.z) ? fragPosLightClip.x : fragPosLightClip.z) :
			((dimDot.y > dimDot.z) ? fragPosLightClip.y : fragPosLightClip.z);

		const float4 pointUVW = float4(-toLightDir, light_i);
		const float pointDepth = ShadowCubemaps.SampleLevel(Sampler, pointUVW, 0).x;
		const float pointResult = pointDepth + EPSILON > fragDepth ? 1.0f : 0.0f;
		const float shadow = saturate(pointResult);


		// Apply lighting
		totalDiffuseLight += diffuseCol * shadow * inverseLightDistSqr;
		totalSpecularLight += specularCol * shadow * inverseLightDistSqr;
	}

	//const float3 result = saturate(col * ((ambient_light.xyz) + totalDiffuseLight) + totalSpecularLight);
	const float3 result = ACESFilm(col * ((ambient_light.xyz) + totalDiffuseLight) + totalSpecularLight);
	BackBufferUAV[DTid.xy] = float4(lerp(result, reflection, reflectivity), 1.0f);
}