#include "DebugData.h"


DebugData::DebugData()
{
	initialized = false;
	fallback.vertexCount = 0;

	fallback.vShader = nullptr;
	fallback.pShader = nullptr;
	fallback.inputLayout = nullptr;
	fallback.vertexBuffer = nullptr;
	fallback.matrixBuffer = nullptr;
	fallback.lightingBuffer = nullptr;
	fallback.texture2D = nullptr;
	fallback.resourceView = nullptr;
	fallback.samplerState = nullptr;
}

DebugData::~DebugData()
{
	if (!initialized)
		return;

	if (fallback.samplerState != nullptr)
		fallback.samplerState->Release();
	if (fallback.resourceView != nullptr)
		fallback.resourceView->Release();
	if (fallback.texture2D != nullptr)
		fallback.texture2D->Release();
	if (fallback.lightingBuffer != nullptr)
		fallback.lightingBuffer->Release();
	if (fallback.matrixBuffer != nullptr)
		fallback.matrixBuffer->Release();
	if (fallback.vertexBuffer != nullptr)
		fallback.vertexBuffer->Release();
	if (fallback.inputLayout != nullptr)
		fallback.inputLayout->Release();
	if (fallback.pShader != nullptr)
		fallback.pShader->Release();
	if (fallback.vShader != nullptr)
		fallback.vShader->Release();
}


bool DebugData::Deinitialize()
{
	if (!initialized)
		return false;

	if (fallback.samplerState != nullptr)
		fallback.samplerState->Release();
	fallback.samplerState = nullptr;

	if (fallback.resourceView != nullptr)
		fallback.resourceView->Release();
	fallback.resourceView = nullptr;

	if (fallback.texture2D != nullptr)
		fallback.texture2D->Release();
	fallback.texture2D = nullptr;

	if (fallback.lightingBuffer != nullptr)
		fallback.lightingBuffer->Release();
	fallback.lightingBuffer = nullptr;

	if (fallback.matrixBuffer != nullptr)
		fallback.matrixBuffer->Release();
	fallback.matrixBuffer = nullptr;

	if (fallback.vertexBuffer != nullptr)
		fallback.vertexBuffer->Release();
	fallback.vertexBuffer = nullptr;

	if (fallback.inputLayout != nullptr)
		fallback.inputLayout->Release();
	fallback.inputLayout = nullptr;

	if (fallback.pShader != nullptr)
		fallback.pShader->Release();
	fallback.pShader = nullptr;

	if (fallback.vShader != nullptr)
		fallback.vShader->Release();
	fallback.vShader = nullptr;

	initialized = false;
	return true;
}

bool DebugData::Initialize(ID3D11Device *device)
{
	if (initialized)
		return false;

	SimpleVertex mesh[] = {
		// -Z
		{{-0.5f, -0.5f, -0.5f}, { 0,  0, -1}, {0, 1}},
		{{-0.5f,  0.5f, -0.5f}, { 0,  0, -1}, {0, 0}},
		{{ 0.5f, -0.5f, -0.5f}, { 0,  0, -1}, {1, 1}},

		{{ 0.5f, -0.5f, -0.5f}, { 0,  0, -1}, {1, 1}},
		{{-0.5f,  0.5f, -0.5f}, { 0,  0, -1}, {0, 0}},
		{{ 0.5f,  0.5f, -0.5f}, { 0,  0, -1}, {1, 0}},

		// +Z
		{{ 0.5f, -0.5f,  0.5f}, { 0,  0,  1}, {0, 1}},
		{{ 0.5f,  0.5f,  0.5f}, { 0,  0,  1}, {0, 0}},
		{{-0.5f, -0.5f,  0.5f}, { 0,  0,  1}, {1, 1}},

		{{-0.5f, -0.5f,  0.5f}, { 0,  0,  1}, {1, 1}},
		{{ 0.5f,  0.5f,  0.5f}, { 0,  0,  1}, {0, 0}},
		{{-0.5f,  0.5f,  0.5f}, { 0,  0,  1}, {1, 0}},


		// -Y
		{{-0.5f, -0.5f,  0.5f}, { 0, -1,  0}, {0, 1}},
		{{-0.5f, -0.5f, -0.5f}, { 0, -1,  0}, {0, 0}},
		{{ 0.5f, -0.5f,  0.5f}, { 0, -1,  0}, {1, 1}},

		{{ 0.5f, -0.5f,  0.5f}, { 0, -1,  0}, {1, 1}},
		{{-0.5f, -0.5f, -0.5f}, { 0, -1,  0}, {0, 0}},
		{{ 0.5f, -0.5f, -0.5f}, { 0, -1,  0}, {1, 0}},

		// +Y
		{{-0.5f,  0.5f, -0.5f}, { 0,  1,  0}, {0, 1}},
		{{-0.5f,  0.5f,  0.5f}, { 0,  1,  0}, {0, 0}},
		{{ 0.5f,  0.5f, -0.5f}, { 0,  1,  0}, {1, 1}},

		{{ 0.5f,  0.5f, -0.5f}, { 0,  1,  0}, {1, 1}},
		{{-0.5f,  0.5f,  0.5f}, { 0,  1,  0}, {0, 0}},
		{{ 0.5f,  0.5f,  0.5f}, { 0,  1,  0}, {1, 0}},


		// -X
		{{-0.5f, -0.5f,  0.5f}, {-1,  0,  0}, {0, 1}},
		{{-0.5f,  0.5f,  0.5f}, {-1,  0,  0}, {0, 0}},
		{{-0.5f, -0.5f, -0.5f}, {-1,  0,  0}, {1, 1}},

		{{-0.5f, -0.5f, -0.5f}, {-1,  0,  0}, {1, 1}},
		{{-0.5f,  0.5f,  0.5f}, {-1,  0,  0}, {0, 0}},
		{{-0.5f,  0.5f, -0.5f}, {-1,  0,  0}, {1, 0}},

		// +X
		{{ 0.5f, -0.5f, -0.5f}, { 1,  0,  0}, {0, 1}},
		{{ 0.5f,  0.5f, -0.5f}, { 1,  0,  0}, {0, 0}},
		{{ 0.5f, -0.5f,  0.5f}, { 1,  0,  0}, {1, 1}},

		{{ 0.5f, -0.5f,  0.5f}, { 1,  0,  0}, {1, 1}},
		{{ 0.5f,  0.5f, -0.5f}, { 1,  0,  0}, {0, 0}},
		{{ 0.5f,  0.5f,  0.5f}, { 1,  0,  0}, {1, 0}},
	};
	fallback.vertexCount = sizeof(mesh) / sizeof(SimpleVertex);

	if (!SetupPipeline(device,
		fallback.vertexBuffer, fallback.matrixBuffer, fallback.lightingBuffer,
		fallback.texture2D, fallback.resourceView, fallback.samplerState,
		fallback.vShader, fallback.pShader, fallback.inputLayout,
		mesh, fallback.vertexCount))
		return false;

	initialized = true;
	return true;
}


bool DebugData::Update(ID3D11Device *device, ID3D11DeviceContext *context, const UINT width, const UINT height, const Time &time, const XMVECTOR &camPos, const XMVECTOR &camDir)
{
	if (!initialized)
		return false;

	XMMATRIX viewProjMatrix =
		XMMatrixLookAtLH(camPos, camPos + camDir, { 0, 1, 0 }) *
		XMMatrixPerspectiveFovLH(50.0f * 0.0174533f, (float)width / height, 0.1f, 10.0f);

	XMMATRIX worldMatrix =
		XMMatrixRotationRollPitchYaw(time.time * 0.76274f, -time.time * 0.3416f, time.time * 0.5384f) *
		XMMatrixTranslation(0.0f, 0.0f, 3.0f);

	XMStoreFloat4x4(&fallback.matrixBufferData.viewProjMatrix, XMMatrixTranspose(viewProjMatrix));
	XMStoreFloat4x4(&fallback.matrixBufferData.worldMatrix, XMMatrixTranspose(worldMatrix));

	if (!UpdateMatrixBuffer(context, fallback.matrixBuffer, fallback.matrixBufferData))
		return false;

	if (!UpdateLightingBuffer(context, fallback.lightingBuffer, fallback.lightingBufferData))
		return false;

	return true;
}

bool DebugData::Draw(ID3D11DeviceContext *context, UINT &vertexCount, ID3D11RenderTargetView *rtv, ID3D11DepthStencilView *dsView, D3D11_VIEWPORT &viewport)
{
	constexpr UINT stride = sizeof(SimpleVertex);
	constexpr UINT offset = 0;

	context->IASetVertexBuffers(0, 1, &fallback.vertexBuffer, &stride, &offset);
	context->IASetInputLayout(fallback.inputLayout);
	context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->VSSetShader(fallback.vShader, nullptr, 0);
	context->VSSetConstantBuffers(0, 1, &fallback.matrixBuffer);

	context->PSSetShader(fallback.pShader, nullptr, 0);
	context->PSSetConstantBuffers(0, 1, &fallback.lightingBuffer);
	context->PSSetShaderResources(0, 1, &fallback.resourceView);
	context->PSSetSamplers(0, 1, &fallback.samplerState);

	vertexCount += fallback.vertexCount;
	return true;
}
