
static const float EPSILON = 0.00005f;

Texture2D Texture		: register(t0);
Texture2D NormalMap		: register(t1);
Texture2D SpecularMap	: register(t2);

sampler Sampler			: register(s0);


cbuffer GlobalLight : register(b0)
{
	float4 ambient_light;
};

cbuffer CameraData : register(b1)
{
	float4 cam_position;
};

cbuffer MaterialProperties : register(b2)
{
	int sampleNormal; // Use normal map if greater than zero.
	int sampleSpecular; // Use specular map if greater than zero.
	float padding[2];
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


struct PixelShaderInput
{
	float4 position			: SV_POSITION;
	float4 world_position	: POSITION;
	float3 normal			: NORMAL;
	float3 tangent			: TANGENT;
	float2 tex_coord		: TEXCOORD;
};

float4 main(PixelShaderInput input) : SV_TARGET
{	
	float4 col = Texture.Sample(Sampler, input.tex_coord);
	
	const float3 bitangent = normalize(cross(input.normal, input.tangent));
	const float3 normal = (sampleNormal > 0)
		? mul(NormalMap.Sample(Sampler, input.tex_coord).xyz * 2.0f - float3(1.0f, 1.0f, 1.0f), float3x3(input.tangent, bitangent, input.normal))
		: input.normal;

	const float specularity = (sampleSpecular > 0)
		? (1.0f / pow(1.0f + EPSILON - SpecularMap.Sample(Sampler, input.tex_coord).x, 1.5f))
		: 0.0f;

	const float3 viewDir = normalize(cam_position.xyz - input.world_position.xyz);

	float3 totalDiffuseLight = float3(0.0f, 0.0f, 0.0f);
	float3 totalSpecularLight = float3(0.0f, 0.0f, 0.0f);


	uint spotLightCount, spotWidth, spotHeight, _;
	SpotLights.GetDimensions(spotLightCount, _);
	SpotShadowMaps.GetDimensions(0, spotWidth, spotHeight, _, _);

	const float
		spotDX = 1.0f / (float)spotWidth,
		spotDY = 1.0f / (float)spotHeight;

	// Per-spotlight calculations
	for (uint spotlight_i = 0; spotlight_i < spotLightCount; spotlight_i++)
	{
		// Prerequisite variables
		const SpotLight light = SpotLights[spotlight_i];

		const float3
			toLight = light.position - input.world_position.xyz,
			toLightDir = normalize(toLight),
			halfwayDir = normalize(toLightDir + viewDir);
		
		const float
			inverseLightDistSqr = 1.0f / (1.0f + (pow(toLight.x * light.falloff, 2) + pow(toLight.y * light.falloff, 2) + pow(toLight.z * light.falloff, 2))),
			maxOffsetAngle = light.angle * 0.5f,
			lightDirOffset = dot(-toLightDir, light.direction),
			offsetAngle = saturate(1.0f - (acos(lightDirOffset) / maxOffsetAngle));
		

		// Calculate Blinn-Phong shading
		const float3 diffuseCol = light.color.xyz * max(abs(dot(normal, toLightDir)), 0.0f);
		
		const float specFactor = pow(saturate(abs(dot(normal, halfwayDir))), specularity);
		const float3 specularCol = specularity * smoothstep(0.0f, 1.0f, specFactor) * float3(1.0f, 1.0f, 1.0f);


		// Calculate shadow projection
		const float4 fragPosLightClip = mul(float4(input.world_position.xyz, 1.0f), light.vp_matrix);
		const float3 fragPosLightNDC = fragPosLightClip.xyz / fragPosLightClip.w;
		
		const bool isInsideFrustum = (
			fragPosLightNDC.x > -1.0f && fragPosLightNDC.x < 1.0f &&
			fragPosLightNDC.y > -1.0f && fragPosLightNDC.y < 1.0f
		);

		const float3
			spotUV00 = float3((fragPosLightNDC.x * 0.5f) + 0.5f, (fragPosLightNDC.y * -0.5f) + 0.5f, spotlight_i),
			spotUV01 = spotUV00 + float3(0.0f, spotDY, 0.0f),
			spotUV10 = spotUV00 + float3(spotDX, 0.0f, 0.0f),
			spotUV11 = spotUV00 + float3(spotDX, spotDY, 0.0f);

		const float
			spotDepth00 = SpotShadowMaps.SampleLevel(Sampler, spotUV00, 0).x,
			spotDepth01 = SpotShadowMaps.SampleLevel(Sampler, spotUV01, 0).x,
			spotDepth10 = SpotShadowMaps.SampleLevel(Sampler, spotUV10, 0).x,
			spotDepth11 = SpotShadowMaps.SampleLevel(Sampler, spotUV11, 0).x;

		const float
			spotResult00 = spotDepth00 + EPSILON > fragPosLightNDC.z ? 1.0f : 0.0f,
			spotResult01 = spotDepth01 + EPSILON > fragPosLightNDC.z ? 1.0f : 0.0f,
			spotResult10 = spotDepth10 + EPSILON > fragPosLightNDC.z ? 1.0f : 0.0f,
			spotResult11 = spotDepth11 + EPSILON > fragPosLightNDC.z ? 1.0f : 0.0f;

		const float2
			texelPos = spotUV00.xy * (float)spotWidth,
			fracTex = frac(texelPos);
		
		const float shadow = isInsideFrustum * saturate(offsetAngle * lerp(
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
	PointShadowMaps.GetDimensions(0, pointWidth, pointHeight, _, _);

	const float
		pointDX = 1.0f / (float)pointWidth,
		pointDY = 1.0f / (float)pointHeight;

	// Per-pointlight calculations
	for (uint pointlight_i = 0; pointlight_i < pointlightCount; pointlight_i++)
	{
		// Prerequisite variables
		const PointLight light = PointLights[pointlight_i];

		const float3
			toLight = light.position - input.world_position.xyz,
			toLightDir = normalize(toLight),
			halfwayDir = normalize(toLightDir + viewDir);
		
		const float inverseLightDistSqr = 1.0f / (1.0f + (
			pow(toLight.x * light.falloff, 2) + 
			pow(toLight.y * light.falloff, 2) + 
			pow(toLight.z * light.falloff, 2)
		));
		

		// Calculate Blinn-Phong shading
		const float3 diffuseCol = light.color.xyz * max(abs(dot(normal, toLightDir)), 0.0f);
		
		const float specFactor = pow(saturate(abs(dot(normal, halfwayDir))), specularity);
		const float3 specularCol = specularity * smoothstep(0.0f, 1.0f, specFactor) * float3(1.0f, 1.0f, 1.0f);


		// Calculate shadow projection
		const float4 fragPosLightClip = mul(float4(input.world_position.xyz, 1.0f), light.vp_matrix);
		const float3 fragPosLightNDC = fragPosLightClip.xyz / fragPosLightClip.w;
		
		const bool isInsideFrustum = (
			fragPosLightNDC.x > -1.0f && fragPosLightNDC.x < 1.0f &&
			fragPosLightNDC.y > -1.0f && fragPosLightNDC.y < 1.0f
		);

		const float3
			pointUV00 = float3((fragPosLightNDC.x * 0.5f) + 0.5f, (fragPosLightNDC.y * -0.5f) + 0.5f, pointlight_i),
			pointUV01 = pointUV00 + float3(0.0f, pointDY, 0.0f),
			pointUV10 = pointUV00 + float3(pointDX, 0.0f, 0.0f),
			pointUV11 = pointUV00 + float3(pointDX, pointDY, 0.0f);

		const float
			pointDepth00 = PointShadowMaps.SampleLevel(Sampler, pointUV00, 0).x,
			pointDepth01 = PointShadowMaps.SampleLevel(Sampler, pointUV01, 0).x,
			pointDepth10 = PointShadowMaps.SampleLevel(Sampler, pointUV10, 0).x,
			pointDepth11 = PointShadowMaps.SampleLevel(Sampler, pointUV11, 0).x;

		const float
			pointResult00 = pointDepth00 + EPSILON > fragPosLightNDC.z ? 1.0f : 0.0f,
			pointResult01 = pointDepth01 + EPSILON > fragPosLightNDC.z ? 1.0f : 0.0f,
			pointResult10 = pointDepth10 + EPSILON > fragPosLightNDC.z ? 1.0f : 0.0f,
			pointResult11 = pointDepth11 + EPSILON > fragPosLightNDC.z ? 1.0f : 0.0f;

		const float2
			texelPos = pointUV00.xy * (float)pointWidth,
			fracTex = frac(texelPos);
		
		const float shadow = isInsideFrustum * saturate(lerp(
			lerp(pointResult00, pointResult10, fracTex.x),
			lerp(pointResult01, pointResult11, fracTex.x),
			fracTex.y)
		);

		// Apply lighting
		totalDiffuseLight += diffuseCol * shadow * inverseLightDistSqr;
		totalSpecularLight += specularCol * shadow * inverseLightDistSqr;
	}


	//const float3 result = saturate(col.xyz * (ambient_light.xyz + totalDiffuseLight + totalSpecularLight));
	const float3 result = ACESFilm(col.xyz * ((ambient_light.xyz) + totalDiffuseLight + totalSpecularLight));
	return float4(result, col.w);
}



