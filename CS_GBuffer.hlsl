
RWTexture2D<unorm float4> BackBufferUAV : register(u0);

Texture2D<float4> GBuffer : register(t0);


[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
	const float4 result = GBuffer[DTid.xy];
	BackBufferUAV[DTid.xy] = saturate(result);
}