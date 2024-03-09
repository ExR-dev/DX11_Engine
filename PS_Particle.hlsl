
struct PixelShaderInput
{
	/*float4 world_position : POSITION;
	float3 normal : NORMAL;
	float3 color : COLOR;*/
	float4 position : SV_POSITION;
	float4 world_position : POSITION;
	float3 normal : NORMAL;
	float2 tex_coord : TEXCOORD;
};

struct PixelShaderOutput
{
	float4 position : SV_Target0;
	float4 color : SV_Target1;
	float4 normal : SV_Target2;
};


PixelShaderOutput main(PixelShaderInput input)
{	
	PixelShaderOutput output;

	output.position = input.world_position;
	//output.color = float4(input.color, 1);
	output.color = float4(1, 0, 0, 1);
	output.normal = float4(input.normal, 1.0f);

	return output;
}