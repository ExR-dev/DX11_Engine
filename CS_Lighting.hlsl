
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

		const float shadowMapDepth = ShadowMaps.SampleLevel(ShadowMapSampler, shadowMapUV, 0).x + EPSILON;
		const float shadowFactor = shadowMapDepth < fragPosNDC.z ? 0.0f : 1.0f;

		// Apply lighting
		//totalSpecularLight += specCol * centerOffset * shadowFactor;
		//totalDiffuseLight += diffCol * centerOffset * shadowFactor;
		totalDiffuseLight += shadowFactor;
		

		/*float4 newLightPos = mul(float4(lightPos, 1.0f), light.vp_matrix);

		newLightPos.xy /= newLightPos.w;

		float3 smTex = float3(0.5f * newLightPos.x + 0.5f, -0.5f * newLightPos.y + 0.5f, lightIndex);
		
		float depth = newLightPos.z / newLightPos.w;

		const float dx = 1.0f / 1024.0f;
		const float s0 = (ShadowMaps.SampleLevel(ShadowMapSampler, smTex + float3(0.0f, 0.0f, 0.0f), 0).x + EPSILON < depth) ? 0.0f : 1.0f;
		const float s1 = (ShadowMaps.SampleLevel(ShadowMapSampler, smTex + float3(dx, 0.0f, 0.0f), 0).x + EPSILON < depth) ? 0.0f : 1.0f;
		const float s2 = (ShadowMaps.SampleLevel(ShadowMapSampler, smTex + float3(0.0f, dx, 0.0f), 0).x + EPSILON < depth) ? 0.0f : 1.0f;
		const float s3 = (ShadowMaps.SampleLevel(ShadowMapSampler, smTex + float3(dx, dx, 0.0f), 0).x + EPSILON < depth) ? 0.0f : 1.0f;
	
		// Transform shadow map UV coord to texel space
		float2 texelPos = smTex.xy * 1024.0f;
		float2 fractex = frac(texelPos);
		// lerp (linear interpolation)
		float shadowCoeff = lerp( lerp( s0, s1, fractex.x ), lerp( s2, s3, fractex.x ), fractex.y );
		//totalDiffuseLight += shadowCoeff;
		totalDiffuseLight += s0;*/

	}

	const float3 result = col * saturate((ambient_light.xyz * ambient_light.w) + totalDiffuseLight) + totalSpecularLight;
	BackBufferUAV[DTid.xy] = float4(saturate(result), 1.0f);
	//BackBufferUAV[DTid.xy] = float4((norm + float3(1,1,1)) * 0.5f, 1.0f);
}