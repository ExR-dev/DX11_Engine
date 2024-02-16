#pragma once

#include "PipelineHelper.h"
#include "Time.h"
#include "Content.h"


struct DebugRenderData
{
	//UINT vertexCount;

	ID3D11VertexShader *vShader;
	ID3D11PixelShader *pShader;
	ID3D11InputLayout *inputLayout;
	//ID3D11Buffer *vertexBuffer;
	ID3D11Buffer *matrixBuffer;
	ID3D11Buffer *lightingBuffer;
	ID3D11Texture2D *texture2D;
	ID3D11ShaderResourceView *resourceView;
	ID3D11SamplerState *samplerState;

	MeshD3D11 *mesh;

	MatrixBufferData matrixBufferData = { };
	LightingBufferData lightingBufferData = {
		{0.0f, 0.0f, 0.0f, 1.0f}, // Camera position
		{0.5f, 2.0f, 2.5f, 1.0f}, // Light position

		{0.75f, 0.9f, 1.0f, 0.05f}, // Ambient
		{1.0f, 1.0f, 1.0f, 5.0f}, // Diffuse
		{10.0f, 10.0f, 10.0f, 128.0f}, // Specular
	};
};


class DebugObject
{
private:
	bool _initialized;
	DebugRenderData _renderData;

public:
	DebugObject();
	~DebugObject();

	bool Initialize(ID3D11Device *device, MeshD3D11 *meshRef);
	bool Initialize(ID3D11Device *device, MeshD3D11 *meshRef, SimpleVertex *mesh, UINT vertexCount);
	bool Uninitialize();

	bool SetVPM(ID3D11DeviceContext *context, const float fov, const float aspect, const float nearPlane, const float farPlane, const XMVECTOR &camPos, const XMVECTOR &camDir);
	bool SetWM(ID3D11DeviceContext *context, const XMVECTOR &pos, const XMVECTOR &rot, const XMVECTOR &scale);

	bool Update(ID3D11DeviceContext *context, const Time &time);
	bool Render(ID3D11DeviceContext *context);
};
