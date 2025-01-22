
static const float EPSILON = 0.00005f;
static const float NORMAL_OFFSET = 0.005f;

RWTexture2DArray<unorm float4> TargetUAV : register(u0);

Texture2D PositionGBuffer	: register(t0); // w is specular r
Texture2D NormalGBuffer		: register(t1); // w is specular g
Texture2D AmbientGBuffer	: register(t2); // w is specular b
Texture2D DiffuseGBuffer	: register(t3); // w is reflectivity

sampler Sampler : register(s0);


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
	int orthographic;
};

StructuredBuffer<SpotLight> SpotLights : register(t4);
Texture2DArray<float> SpotShadowMaps : register(t5);

struct PointLight
{
	float4x4 vp_matrix;
	float3 position;
	float3 color;
	float falloff;
	float padding;
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

void BlinnPhong(float3 toLightDir, float3 viewDir, float3 normal, float3 lightCol, float specularity, out float3 diffuse, out float3 specular)
{
	const float3 halfwayDir = normalize(toLightDir + viewDir);
		
	float directionScalar = max(dot(normal, toLightDir), 0.0f);
	diffuse = lightCol * directionScalar;
	
	const float specFactor = pow(max(dot(normal, halfwayDir), 0.0f), specularity);
	specular = lightCol * directionScalar * smoothstep(0.0f, 1.0f, specFactor);
}

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	const float4
		positionGBuf = PositionGBuffer[DTid.xy],
		normalGBuf = NormalGBuffer[DTid.xy],
		ambientGBuf = AmbientGBuffer[DTid.xy],
		diffuseGBuf = DiffuseGBuffer[DTid.xy];
	
	float specularity;
	modf(diffuseGBuf.w, specularity);
	const float3
		pos = positionGBuf.xyz,
		diffuseCol = diffuseGBuf.xyz,
		ambientCol = ambientGBuf.xyz,
		specularCol = float3(positionGBuf.w, normalGBuf.w, ambientGBuf.w),
		norm = normalize(normalGBuf.xyz),
		viewDir = normalize(cam_position.xyz - pos);
		
	float3 totalDiffuseLight = float3(0.01f, 0.0175f, 0.02f); // Scene-wide ambient light
	float3 totalSpecularLight = float3(0.0f, 0.0f, 0.0f);


	uint spotlightCount, _;
	SpotLights.GetDimensions(spotlightCount, _);

	// Per-spotlight calculations
	for (uint spotlight_i = 0; spotlight_i < spotlightCount; spotlight_i++)
	{
		// Prerequisite variables
		const SpotLight light = SpotLights[spotlight_i];

		const float3
			toLight = light.position - pos,
			toLightDir = normalize(toLight);

		const float
			projectedDist = dot(light.direction, -toLight),
			inverseLightDistSqr = light.orthographic > 0 ?
			(projectedDist > 0.0f ? 1.0f : 0.0f) * (1.0f / (1.0f + pow(projectedDist, 2))) :
			1.0f / (1.0f + (pow(toLight.x * light.falloff, 2) + pow(toLight.y * light.falloff, 2) + pow(toLight.z * light.falloff, 2)));

		const float offsetAngle = saturate(1.0f - (
			light.orthographic > 0 ? 
			length(cross(light.direction, toLight)) : 
			acos(dot(-toLightDir, light.direction))
			) / (light.angle * 0.5f));
		
		
		float3 diffuseLightCol, specularLightCol;
		BlinnPhong(toLightDir, viewDir, norm, light.color.xyz, specularity, diffuseLightCol, specularLightCol);
		specularLightCol *= specularCol;


		// Calculate shadow projection
		const float4 fragPosLightClip = mul(float4(pos + norm * NORMAL_OFFSET, 1.0f), light.vp_matrix);
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
		totalDiffuseLight += diffuseLightCol * shadow * inverseLightDistSqr;
		totalSpecularLight += specularLightCol * shadow * inverseLightDistSqr;
	}


	uint dirlightCount;
	DirLights.GetDimensions(dirlightCount, _);

	// Per-directional light calculations
	for (uint dirlight_i = 0; dirlight_i < dirlightCount; dirlight_i++)
	{
		// Prerequisite variables
		const DirLight light = DirLights[dirlight_i];

		const float3 toLightDir = normalize(-light.direction);


		float3 diffuseLightCol, specularLightCol;
		BlinnPhong(toLightDir, viewDir, norm, light.color.xyz, specularity, diffuseLightCol, specularLightCol);
		specularLightCol *= specularCol;


		// Calculate shadow projection
		const float4 fragPosLightClip = mul(float4(pos + norm * NORMAL_OFFSET, 1.0f), light.vp_matrix);
		const float3 fragPosLightNDC = fragPosLightClip.xyz / fragPosLightClip.w;

		const float3 dirUV = float3((fragPosLightNDC.x * 0.5f) + 0.5f, (fragPosLightNDC.y * -0.5f) + 0.5f, dirlight_i);
		const float dirDepth = DirShadowMaps.SampleLevel(Sampler, dirUV, 0).x;
		const float dirResult = dirDepth - EPSILON < fragPosLightNDC.z ? 1.0f : 0.0f;
		const float shadow = dirResult;


		// Apply lighting
		totalDiffuseLight += diffuseLightCol * shadow;
		totalSpecularLight += specularLightCol * shadow;
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
			toLightDir = normalize(toLight);
		
		const float inverseLightDistSqr = 1.0f / (1.0f + (
			pow(toLight.x * light.falloff, 2) + 
			pow(toLight.y * light.falloff, 2) + 
			pow(toLight.z * light.falloff, 2)
		));
		

		float3 diffuseLightCol, specularLightCol;
		BlinnPhong(toLightDir, viewDir, norm, light.color.xyz, specularity, diffuseLightCol, specularLightCol);
		specularLightCol *= specularCol;


		// Calculate shadow projection
		const float4 fragPosLightClip = mul(float4(pos + norm * NORMAL_OFFSET, 1.0f), light.vp_matrix);
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
		totalDiffuseLight += diffuseLightCol * shadow * inverseLightDistSqr;
		totalSpecularLight += specularLightCol * shadow * inverseLightDistSqr;
	}

	
	totalDiffuseLight *= diffuseCol;
	const float3 result = ACESFilm(ambientCol + totalDiffuseLight + totalSpecularLight);
	TargetUAV[uint3(DTid.xy, 0)] = float4(result, 1.0f);
}