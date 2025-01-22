
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

struct DirLight
{
	float4x4 vp_matrix;
	float3 direction;
	float3 color;

	float padding[2];
};

StructuredBuffer<DirLight> DirLights : register(t8);
Texture2DArray<float> DirShadowMaps : register(t9);


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
	
	float3 totalDiffuseLight = float3(0.01f, 0.0175f, 0.02f); // Scene-wide ambient light
	float3 totalSpecularLight = float3(0.0f, 0.0f, 0.0f);


	uint spotLightCount, _;
	SpotLights.GetDimensions(spotLightCount, _);

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
		const float3 diffuseCol = light.color.xyz * max((dot(normal, toLightDir) + 1.5f) / 2.5f, 0.0f);
		
		const float specFactor = pow(max(dot(normal, halfwayDir), 0.0f), specularity);
		const float3 specularCol = specularity * smoothstep(0.0f, 1.0f, specFactor) * float3(1.0f, 1.0f, 1.0f);


		// Calculate shadow projection
		const float4 fragPosLightClip = mul(float4(input.world_position.xyz, 1.0f), light.vp_matrix);
		const float3 fragPosLightNDC = fragPosLightClip.xyz / fragPosLightClip.w;
		
		const bool isInsideFrustum = (
			fragPosLightNDC.x > -1.0f && fragPosLightNDC.x < 1.0f &&
			fragPosLightNDC.y > -1.0f && fragPosLightNDC.y < 1.0f
		);

		const float3 spotUV = float3((fragPosLightNDC.x * 0.5f) + 0.5f, (fragPosLightNDC.y * -0.5f) + 0.5f, spotlight_i);
		const float spotDepth = SpotShadowMaps.SampleLevel(Sampler, spotUV, 0).x;
		const float spotResult = spotDepth - EPSILON < fragPosLightNDC.z ? 1.0f : 0.0f;
		const float shadow = isInsideFrustum * saturate(offsetAngle * spotResult);

		// Apply lighting
		totalDiffuseLight += diffuseCol * shadow * inverseLightDistSqr;
		totalSpecularLight += specularCol * shadow * inverseLightDistSqr;
	}

	
	uint dirLightCount;
	DirLights.GetDimensions(dirLightCount, _);

	// Per-directional light calculations
	for (uint dirlight_i = 0; dirlight_i < dirLightCount; dirlight_i++)
	{
		// Prerequisite variables
		const DirLight light = DirLights[dirlight_i];

		const float3
			toLightDir = normalize(-light.direction),
			halfwayDir = normalize(toLightDir + viewDir);
		
		
		// Calculate Blinn-Phong shading
		const float3 diffuseCol = light.color.xyz * max((dot(normal, toLightDir) + 1.5f) / 2.5f, 0.0f);
		
		const float specFactor = pow(max(dot(normal, halfwayDir), 0.0f), specularity);
		const float3 specularCol = specularity * smoothstep(0.0f, 1.0f, specFactor) * float3(1.0f, 1.0f, 1.0f);


		// Calculate shadow projection
		const float4 fragPosLightClip = mul(float4(input.world_position.xyz, 1.0f), light.vp_matrix);
		const float3 fragPosLightNDC = fragPosLightClip.xyz / fragPosLightClip.w;
		
		const float3 dirUV = float3((fragPosLightNDC.x * 0.5f) + 0.5f, (fragPosLightNDC.y * -0.5f) + 0.5f, dirlight_i);
		const float dirDepth = DirShadowMaps.SampleLevel(Sampler, dirUV, 0).x;
		const float dirResult = dirDepth - EPSILON < fragPosLightNDC.z ? 1.0f : 0.0f;
		const float shadow = dirResult;


		// Apply lighting
		totalDiffuseLight += diffuseCol * shadow;
		totalSpecularLight += specularCol * shadow;
	}
	
	
	uint pointlightCount;
	PointLights.GetDimensions(pointlightCount, _);
	
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
		const float3 diffuseCol = light.color.xyz * max((dot(normal, toLightDir) + 1.5f) / 2.5f, 0.0f);
		
		const float specFactor = pow(max(dot(normal, halfwayDir), 0.0f), specularity);
		const float3 specularCol = specularity * smoothstep(0.0f, 1.0f, specFactor) * float3(1.0f, 1.0f, 1.0f);


		// Calculate shadow projection
		const float4 fragPosLightClip = mul(float4(input.world_position.xyz, 1.0f), light.vp_matrix);
		const float3 fragPosLightNDC = fragPosLightClip.xyz / fragPosLightClip.w;
		
		const bool isInsideFrustum = (
			fragPosLightNDC.x > -1.0f && fragPosLightNDC.x < 1.0f &&
			fragPosLightNDC.y > -1.0f && fragPosLightNDC.y < 1.0f
		);

		const float3 pointUV = float3((fragPosLightNDC.x * 0.5f) + 0.5f, (fragPosLightNDC.y * -0.5f) + 0.5f, pointlight_i);
		const float pointDepth = PointShadowMaps.SampleLevel(Sampler, pointUV, 0).x;
		const float pointResult = pointDepth - EPSILON < fragPosLightNDC.z ? 1.0f : 0.0f;
		const float shadow = isInsideFrustum * saturate(pointResult);


		// Apply lighting
		totalDiffuseLight += diffuseCol * shadow * inverseLightDistSqr;
		totalSpecularLight += specularCol * shadow * inverseLightDistSqr;
	}
	

	const float3 result = ACESFilm(col.xyz * ((ambient_light.xyz) + totalDiffuseLight + totalSpecularLight));
	return float4(result, col.w);
}



