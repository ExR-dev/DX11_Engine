
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

	uint spotLightCount, smWidth, smHeight, _;
	SpotLights.GetDimensions(spotLightCount, _);
	ShadowMaps.GetDimensions(0, smWidth, smHeight, _, _);

	const float
		smDX = 1.0f / (float)smWidth,
		smDY = 1.0f / (float)smHeight;

	float3 totalDiffuseLight = float3(0.0f, 0.0f, 0.0f);
	float3 totalSpecularLight = float3(0.0f, 0.0f, 0.0f);

	// Per-spotlight calculations
	for (uint light_i = 0; light_i < spotLightCount; light_i++)
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
		const float3 diffuseCol = light.color.xyz * max(abs(dot(normal, toLightDir)), 0.0f);
		
		const float specFactor = pow(saturate(abs(dot(normal, halfwayDir))), specularity);
		const float3 specularCol = specularity * smoothstep(0.0f, 1.0f, specFactor) * float3(1.0f, 1.0f, 1.0f);


		// Calculate shadow projection
		const float4 fragPosLightClip = mul(float4(input.world_position.xyz, 1.0f), light.vp_matrix);
		const float3 fragPosLightNDC = fragPosLightClip.xyz / fragPosLightClip.w;
		
		/*
		const float3 smUV = float3((fragPosLightNDC.x * 0.5f) + 0.5f, (fragPosLightNDC.y * -0.5f) + 0.5f, light_i);
		const float smDepth = ShadowMaps.SampleLevel(Sampler, smUV, 0).x;
		const float smResult = smDepth + EPSILON > fragPosLightNDC.z ? 1.0f : 0.0f;
		const float shadow = saturate(offsetAngle * smResult);
		*/

		const float3
			smUV00 = float3((fragPosLightNDC.x * 0.5f) + 0.5f, (fragPosLightNDC.y * -0.5f) + 0.5f, light_i),
			smUV01 = smUV00 + float3(0.0f, smDY, 0.0f),
			smUV10 = smUV00 + float3(smDX, 0.0f, 0.0f),
			smUV11 = smUV00 + float3(smDX, smDY, 0.0f);

		const float
			smDepth00 = ShadowMaps.SampleLevel(Sampler, smUV00, 0).x,
			smDepth01 = ShadowMaps.SampleLevel(Sampler, smUV01, 0).x,
			smDepth10 = ShadowMaps.SampleLevel(Sampler, smUV10, 0).x,
			smDepth11 = ShadowMaps.SampleLevel(Sampler, smUV11, 0).x;

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

		// Apply lighting
		totalDiffuseLight += diffuseCol * shadow * inverseLightDistSqr;
		totalSpecularLight += specularCol * shadow * inverseLightDistSqr;
	}

	//const float3 result = saturate(col.xyz * (ambient_light.xyz + totalDiffuseLight + totalSpecularLight));
	const float3 result = ACESFilm(col.xyz * ((ambient_light.xyz) + totalDiffuseLight + totalSpecularLight));
	return float4(result, col.w);
}



