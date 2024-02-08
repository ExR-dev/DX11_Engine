#pragma once

#include "PipelineHelper.h"
#include "Time.h"


struct FallbackShaderData
{
	UINT vertexCount;

	ID3D11VertexShader *vShader;
	ID3D11PixelShader *pShader;
	ID3D11InputLayout *inputLayout;
	ID3D11Buffer *vertexBuffer;
	ID3D11Buffer *matrixBuffer;
	ID3D11Buffer *lightingBuffer;
	ID3D11Texture2D *texture2D;
	ID3D11ShaderResourceView *resourceView;
	ID3D11SamplerState *samplerState;

	MatrixBufferData matrixBufferData = { };
	LightingBufferData lightingBufferData = {
		{0.0f, 0.0f, 0.0f, 1.0f}, // Camera position
		{3.0f, 0.5f, -1.5f, 1.0f}, // Light position

		{0.75f, 0.9f, 1.0f, 0.05f}, // Ambient
		{1.0f, 1.0f, 1.0f, 10.0f}, // Diffuse
		{5.0f, 5.0f, 5.0f, 128.0f}, // Specular
	};
};


class DebugData
{
private:
	bool initialized;

public:
	FallbackShaderData fallback;

	DebugData();
	~DebugData();

	bool Deinitialize();
	bool Initialize(ID3D11Device *device);
	bool Update(ID3D11Device *device, ID3D11DeviceContext *context, const UINT width, const UINT height, const Time &time, const XMVECTOR &camPos, const XMVECTOR &camDir);
	bool Draw(ID3D11DeviceContext *context, UINT &vertexCount, ID3D11RenderTargetView *rtv, ID3D11DepthStencilView *dsView, D3D11_VIEWPORT &viewport);
};
