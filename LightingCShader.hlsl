
RWTexture2D<unorm float4> BackBufferUAV : register(u0);

Texture2D<float4> PositionGBuffer : register(t0);
Texture2D<float4> ColorGBuffer : register(t1);
Texture2D<float4> NormalGBuffer : register(t2);


cbuffer LightingData : register(b0)
{
	float4 light_position;
	float4 ambient;
	float4 diffuse;
	float4 specular;
};

cbuffer CameraData : register(b1)
{
	float4 cam_position;
};


[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	const float3 position = PositionGBuffer[DTid.xy].xyz;
	const float3 color = ColorGBuffer[DTid.xy].xyz;
	const float3 normal = normalize(NormalGBuffer[DTid.xy].xyz);
	
	// Prerequisite variables
	const float3 toLight = light_position.xyz - position;
	const float inverseLightDistSqr = 1.0f / (pow(toLight.x, 2) + pow(toLight.y, 2) + pow(toLight.z, 2));

	const float3 lightDir = normalize(toLight);
	const float3 viewDir = normalize(cam_position.xyz - position);
	const float3 halfwayDir = normalize(lightDir + viewDir);
	
	// Lighting types
	const float3 ambCol = color * ambient.xyz * ambient.w;
	
	const float3 diffCol = color * diffuse.xyz * diffuse.w * max(dot(normal, lightDir), 0) * inverseLightDistSqr;
	
	const float specFactor = pow(saturate(dot(normal, halfwayDir)), specular.w);
	const float3 specCol = specular.xyz * smoothstep(0, 1, specFactor * inverseLightDistSqr);
	
	// Apply lighting
	const float3 lighting = ambCol + diffCol + specCol;
	
	BackBufferUAV[DTid.xy] = float4(saturate(lighting), 1.0f);
}