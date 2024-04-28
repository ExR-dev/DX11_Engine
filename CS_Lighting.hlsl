
static const float EPSILON = 0.00005f;
static const float NORMAL_OFFSET = 0.005f;

RWTexture2D<unorm float4> BackBufferUAV : register(u0);

Texture2D PositionGBuffer	: register(t0); // w is specular r
Texture2D NormalGBuffer		: register(t1); // w is specular g
Texture2D AmbientGBuffer	: register(t2); // w is specular b
Texture2D DiffuseGBuffer	: register(t3); // w is reflectivity

sampler Sampler : register(s0);


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

TextureCube EnvironmentCubemap	: register(t10);


cbuffer CameraData : register(b1)
{
	float4 cam_position;
};


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
	const float reflectivity = modf(diffuseGBuf.w, specularity) * 1.010101f;
	const float3
		pos = positionGBuf.xyz,
		diffuseCol = diffuseGBuf.xyz,
		ambientCol = ambientGBuf.xyz,
		specularCol = float3(positionGBuf.w, normalGBuf.w, ambientGBuf.w),
		norm = normalize(normalGBuf.xyz),
		viewDir = normalize(cam_position.xyz - pos);

	const float3 reflection = (reflectivity > 0.05f)
		? EnvironmentCubemap.SampleLevel(Sampler, reflect(viewDir, norm) * float3(-1, 1, 1), 0).xyz
		: float3(0,0,0);

	float3 totalDiffuseLight = float3(0.01f, 0.0175f, 0.02f); // Scene-wide ambient light
	float3 totalSpecularLight = float3(0.0f, 0.0f, 0.0f);


	uint spotlightCount, spotWidth, spotHeight, _u;
	SpotLights.GetDimensions(spotlightCount, _u);
	SpotShadowMaps.GetDimensions(0, spotWidth, spotHeight, _u, _u);

	const float
		spotDX = 1.0f / (float)spotWidth,
		spotDY = 1.0f / (float)spotHeight;

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
			spotResult00 = spotDepth00 - EPSILON < fragPosLightNDC.z ? 1.0f : 0.0f,
			spotResult01 = spotDepth01 - EPSILON < fragPosLightNDC.z ? 1.0f : 0.0f,
			spotResult10 = spotDepth10 - EPSILON < fragPosLightNDC.z ? 1.0f : 0.0f,
			spotResult11 = spotDepth11 - EPSILON < fragPosLightNDC.z ? 1.0f : 0.0f;

		const float2
			texelPos = spotUV00.xy * (float)spotWidth,
			fracTex = frac(texelPos);
		
		const float shadow = isInsideFrustum * saturate(
			offsetAngle * lerp(
				lerp(spotResult00, spotResult10, fracTex.x),
				lerp(spotResult01, spotResult11, fracTex.x),
				fracTex.y)
		);


		// Apply lighting
		totalDiffuseLight += diffuseLightCol * shadow * inverseLightDistSqr;
		totalSpecularLight += specularLightCol * shadow * inverseLightDistSqr;
	}


	uint dirlightCount, dirWidth, dirHeight;
	DirLights.GetDimensions(dirlightCount, _u);
	DirShadowMaps.GetDimensions(0, dirWidth, dirHeight, _u, _u);

	const float
		dirDX = 1.0f / (float)dirWidth,
		dirDY = 1.0f / (float)dirHeight;

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

		const float3
			dirUV00 = float3((fragPosLightNDC.x * 0.5f) + 0.5f, (fragPosLightNDC.y * -0.5f) + 0.5f, dirlight_i),
			dirUV01 = dirUV00 + float3(0.0f, dirDY, 0.0f),
			dirUV10 = dirUV00 + float3(dirDX, 0.0f, 0.0f),
			dirUV11 = dirUV00 + float3(dirDX, dirDY, 0.0f);

		const float
			dirDepth00 = DirShadowMaps.SampleLevel(Sampler, dirUV00, 0).x,
			dirDepth01 = DirShadowMaps.SampleLevel(Sampler, dirUV01, 0).x,
			dirDepth10 = DirShadowMaps.SampleLevel(Sampler, dirUV10, 0).x,
			dirDepth11 = DirShadowMaps.SampleLevel(Sampler, dirUV11, 0).x;

		const float
			dirResult00 = dirDepth00 - EPSILON < fragPosLightNDC.z ? 1.0f : 0.0f,
			dirResult01 = dirDepth01 - EPSILON < fragPosLightNDC.z ? 1.0f : 0.0f,
			dirResult10 = dirDepth10 - EPSILON < fragPosLightNDC.z ? 1.0f : 0.0f,
			dirResult11 = dirDepth11 - EPSILON < fragPosLightNDC.z ? 1.0f : 0.0f;

		const float2
			texelPos = dirUV00.xy * (float)dirWidth,
			fracTex = frac(texelPos);
		
		const float shadow = saturate(lerp(
			lerp(dirResult00, dirResult10, fracTex.x),
			lerp(dirResult01, dirResult11, fracTex.x),
			fracTex.y)
		);


		// Apply lighting
		totalDiffuseLight += diffuseLightCol * shadow;
		totalSpecularLight += specularLightCol * shadow;
	}


	uint pointlightCount, pointWidth, pointHeight;
	PointLights.GetDimensions(pointlightCount, _u);
	PointShadowMaps.GetDimensions(0, pointWidth, pointHeight, _u, _u);

	const float
		pointDX = 1.0f / (float)pointWidth,
		pointDY = 1.0f / (float)pointHeight;

	// Per-pointlight calculations
	for (uint pointlight_i = 0; pointlight_i < pointlightCount; pointlight_i++)
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
			pointResult00 = pointDepth00 - EPSILON < fragPosLightNDC.z ? 1.0f : 0.0f,
			pointResult01 = pointDepth01 - EPSILON < fragPosLightNDC.z ? 1.0f : 0.0f,
			pointResult10 = pointDepth10 - EPSILON < fragPosLightNDC.z ? 1.0f : 0.0f,
			pointResult11 = pointDepth11 - EPSILON < fragPosLightNDC.z ? 1.0f : 0.0f;

		const float2
			texelPos = pointUV00.xy * (float)pointWidth,
			fracTex = frac(texelPos);
		
		const float shadow = isInsideFrustum * saturate(lerp(
			lerp(pointResult00, pointResult10, fracTex.x),
			lerp(pointResult01, pointResult11, fracTex.x),
			fracTex.y)
		);


		// Apply lighting
		totalDiffuseLight += diffuseLightCol * shadow * inverseLightDistSqr;
		totalSpecularLight += specularLightCol * shadow * inverseLightDistSqr;
	}


	totalDiffuseLight *= diffuseCol;
	const float3 result = ACESFilm(ambientCol + totalDiffuseLight + totalSpecularLight);
	BackBufferUAV[DTid.xy] = float4(lerp(result, reflection, reflectivity), 1.0f);
}