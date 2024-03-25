
cbuffer ObjectPosition : register(b0)
{
	float4 objPos;
};

cbuffer CameraPosition : register(b1)
{
	float4 camPos;
};


struct VS_CONTROL_POINT_OUTPUT
{
	float3 wPos		: WORLD_POS;
	float3 normal	: NORMAL;
	float2 uv		: UV;
};

struct HS_CONTROL_POINT_OUTPUT
{
	float3 wPos		: WORLD_POS;
	float3 normal	: NORMAL;
	float2 uv		: UV;
};

struct HS_CONSTANT_DATA_OUTPUT
{
	float EdgeTessFactor[3]	: SV_TessFactor;
	float InsideTessFactor	: SV_InsideTessFactor;
};

#define NUM_CONTROL_POINTS 3

HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint patchID : SV_PrimitiveID)
{
	HS_CONSTANT_DATA_OUTPUT output;

	output.EdgeTessFactor[0] = 
		output.EdgeTessFactor[1] = 
		output.EdgeTessFactor[2] = 
		output.InsideTessFactor = 32;

	return output;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("CalcHSPatchConstants")]
HS_CONTROL_POINT_OUTPUT main( 
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip, 
	uint i : SV_OutputControlPointID,
	uint patchID : SV_PrimitiveID )
{
	HS_CONTROL_POINT_OUTPUT output;

	output.wPos		= ip[i].wPos;
	output.normal	= ip[i].normal;
	output.uv		= ip[i].uv;

	return output;
}
