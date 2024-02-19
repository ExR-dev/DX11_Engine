#pragma once

#include "ConstantBufferD3D11.h"
#include "PipelineHelper.h"
#include "Time.h"
#include "Content.h"
#include "Transform.h"


struct DebugRenderData
{
	// TODO: Refactor
	//ID3D11VertexShader *vShader = nullptr;
	//ID3D11PixelShader *pShader = nullptr;
	ID3D11InputLayout *inputLayout = nullptr;
	//ID3D11Buffer *worldMatrixBuffer = nullptr;
	//ID3D11Buffer *viewProjMatrixBuffer = nullptr;
	//ID3D11Buffer *lightingBuffer = nullptr;
	ID3D11Texture2D *texture2D = nullptr;
	ID3D11ShaderResourceView *resourceView = nullptr;
	ID3D11SamplerState *samplerState = nullptr;

	WorldMatrixBufferData worldMatrixBufferData = { };
	ViewProjMatrixBufferData viewProjMatrixBufferData = { };
	LightingBufferData lightingBufferData = {
		{0.0f, 0.0f, 0.0f, 1.0f}, // Camera position
		{0.5f, 2.0f, 2.5f, 1.0f}, // Light position

		{0.75f, 0.9f, 1.0f, 0.05f}, // Ambient
		{1.0f, 1.0f, 1.0f, 5.0f}, // Diffuse
		{10.0f, 10.0f, 10.0f, 128.0f}, // Specular
	};

	// Refactored
	UINT meshID = CONTENT_LOAD_ERROR;
	UINT vsID = CONTENT_LOAD_ERROR;
	UINT psID = CONTENT_LOAD_ERROR;

	ConstantBufferD3D11 worldMatrixBuffer;
	ConstantBufferD3D11 lightingBuffer;
};


class DebugObject
{
private:
	bool _initialized;
	Transform _transform;
	DebugRenderData _renderData;

public:
	DebugObject();
	~DebugObject();
	DebugObject(const DebugObject &other) = delete;
	DebugObject &operator=(const DebugObject &other) = delete;
	DebugObject(DebugObject &&other) = default;
	DebugObject &operator=(DebugObject &&other) = delete;

	bool Initialize(ID3D11Device *device, UINT meshID, UINT vsID, UINT psID);

	bool SetWM(ID3D11DeviceContext *context) const;

	bool Update(ID3D11DeviceContext *context, const Time &time);
	bool Render(ID3D11DeviceContext *context);
};
