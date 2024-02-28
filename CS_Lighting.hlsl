
static const float PI = 3.14159265f;


RWTexture2D<unorm float4> BackBufferUAV : register(u0);

Texture2D<float4> PositionGBuffer : register(t0);
Texture2D<float4> ColorGBuffer : register(t1);
Texture2D<float4> NormalGBuffer : register(t2);


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


cbuffer CameraData : register(b0)
{
	float4 cam_position;
};


[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	const float3 pos = PositionGBuffer[DTid.xy].xyz;
	const float3 col = ColorGBuffer[DTid.xy].xyz;
	const float3 norm = normalize(NormalGBuffer[DTid.xy].xyz);
	
	const float3 ambientLight = float3(0.02f, 0.02f, 0.02f);
	float3 totalDiffuseLight = float3(0.0f, 0.0f, 0.0f);
	float3 totalSpecularLight = float3(0.0f, 0.0f, 0.0f);

	uint lightCount, lightSize;
	SpotLights.GetDimensions(lightCount, lightSize);

	for (uint lightIndex = 0; lightIndex < lightCount; lightIndex++)
	{
		const SpotLight light = SpotLights[lightIndex];
		const float3 lightPos = light.light_position;
		const float3 lightDir = light.direction;
		const float3 lightCol = light.color;
		const float lightMaxAngle = light.angle / 2.0f;


		
		// Prerequisite variables
		const float3 toLight = lightPos - pos;
		const float inverseLightDistSqr = 1.0f / (pow(toLight.x, 2) + pow(toLight.y, 2) + pow(toLight.z, 2));

		const float3 toLightDir = normalize(toLight);
		const float3 viewDir = normalize(cam_position - pos);
		const float3 halfwayDir = normalize(toLightDir + viewDir);


		const float lightDirOffset = dot(toLightDir, lightDir);

		//float centerOffset = 0.0f;

		float centerOffset = acos(lightDirOffset) / PI;
		float maxCenterOffset = lightMaxAngle / 180.0f;

		centerOffset = saturate(1.0f - (centerOffset / maxCenterOffset));

		
		// Lighting types
		const float3 diffCol = lightCol.xyz * max(dot(norm, toLightDir), 0) * inverseLightDistSqr;
		
		const float specFactor = pow(saturate(dot(norm, halfwayDir)), 1);
		const float3 specCol = float3(1, 1, 1) * smoothstep(0, 1, specFactor * inverseLightDistSqr);
		
		float4 fragPosClip = mul(float4(pos, 1.0f), light.vp_matrix);
		float3 fragPosNDC = fragPosClip.xyz / fragPosClip.w;

		const float3 shadowMapUV = float3((fragPosNDC.x * 0.5f) + 0.5f, (fragPosNDC.y * -0.5f) + 0.5f, lightIndex);

		const float shadowMapDepth = ShadowMaps.SampleLevel(ShadowMapSampler, shadowMapUV, 0).x + 0.005f;
		const float shadowFactor = shadowMapDepth < fragPosNDC.z ? 0.0f : 1.0f;

		// Apply lighting
		totalDiffuseLight += diffCol /** centerOffset*/ * shadowFactor;
		totalSpecularLight += specCol /** centerOffset*/ * shadowFactor;

		/*
		// Calculate the light direction
		const float3 toLight = lightPos - pos;
		const float3 toLightDir = normalize(toLight);

		// Calculate the angle between the light direction and the normal
		const float toLightAngle = dot(toLightDir, -lightDir);


		// Sample the shadow map
		const float shadowMapDepth = ShadowMaps.Sample(ShadowMapSampler, shadowMapUV).r;
		const float lightDepth = lightPos.z;
		const float shadowFactor = shadowMapDepth < lightDepth ? 0.0f : 1.0f;

		// Prerequisite variables
		const float inverseLightDistSqr = 1.0f / (pow(toLight.x, 2) + pow(toLight.y, 2) + pow(toLight.z, 2));

		const float3 viewDir = normalize(cam_position.xyz - pos);
		const float3 halfwayDir = normalize(toLightDir + viewDir);

		const float diffFactor = max(dot(norm, toLightDir), 0);
		const float3 diffCol = lightCol * diffFactor * inverseLightDistSqr;

		const float specFactor = pow(saturate(dot(norm, halfwayDir)), 32);
		const float3 specCol = lightCol * smoothstep(0, 1, specFactor * inverseLightDistSqr);

		// Apply lighting)
		totalDiffuseLight += diffCol * shadowFactor;
		totalSpecularLight += specCol * shadowFactor;
		*/
	}

	const float3 result = col * saturate(ambientLight + totalDiffuseLight) + totalSpecularLight;
	BackBufferUAV[DTid.xy] = float4(saturate(result), 1.0f);
}