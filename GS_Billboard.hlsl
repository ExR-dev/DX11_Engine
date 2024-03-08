
cbuffer CameraData : register(b0)
{
	float4x4 view_projection;
	float4 cam_position;
};

struct GeometryShaderOutput
{
	float4 position : SV_POSITION;
	float4 world_position : POSITION;
	float3 normal : NORMAL;
	float2 tex_coord : TEXCOORD;
};


[maxvertexcount(6)]
void main(
	point float4 input[1] : POSITION, 
	inout TriangleStream<GeometryShaderOutput> output)
{
	const float3
		frontVec = normalize(input[0].xyz - cam_position.xyz),
		rightVec = normalize(cross(frontVec, float3(0, 1, 0))),
		upVec = normalize(cross(frontVec, rightVec));

	GeometryShaderOutput newVertex;
	newVertex.normal = frontVec;

	// Top left
	newVertex.world_position = float4(input[0].xyz - rightVec + upVec, 1.0f); 
	newVertex.position = mul(newVertex.world_position, view_projection);
	newVertex.tex_coord = float2(0, 1);
	output.Append(newVertex);

	// Bottom right
	newVertex.world_position = float4(input[0].xyz + rightVec - upVec, 1.0f); 
	newVertex.position = mul(newVertex.world_position, view_projection);
	newVertex.tex_coord = float2(1, 0);
	output.Append(newVertex);

	// Bottom left
	newVertex.world_position = float4(input[0].xyz - rightVec - upVec, 1.0f); 
	newVertex.position = mul(newVertex.world_position, view_projection);
	newVertex.tex_coord = float2(0, 0);
	output.Append(newVertex);

	output.RestartStrip(); // Done with first triangle
	
	// Top left
	newVertex.world_position = float4(input[0].xyz - rightVec + upVec, 1.0f); 
	newVertex.position = mul(newVertex.world_position, view_projection);
	newVertex.tex_coord = float2(0, 1);
	output.Append(newVertex);

	// Top right
	newVertex.world_position = float4(input[0].xyz + rightVec + upVec, 1.0f); 
	newVertex.position = mul(newVertex.world_position, view_projection);
	newVertex.tex_coord = float2(1, 1);
	output.Append(newVertex);

	// Bottom right
	newVertex.world_position = float4(input[0].xyz + rightVec - upVec, 1.0f); 
	newVertex.position = mul(newVertex.world_position, view_projection);
	newVertex.tex_coord = float2(1, 0);
	output.Append(newVertex);
}