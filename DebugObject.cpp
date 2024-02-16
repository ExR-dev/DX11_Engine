#include "DebugObject.h"

#include <iostream>
#include "ErrMsg.h"


DebugObject::DebugObject()
{
	_initialized = false;
	//_renderData.vertexCount = 0;

	_renderData.vShader = nullptr;
	_renderData.pShader = nullptr;
	_renderData.inputLayout = nullptr;
	//_renderData.vertexBuffer = nullptr;
	_renderData.matrixBuffer = nullptr;
	_renderData.lightingBuffer = nullptr;
	_renderData.texture2D = nullptr;
	_renderData.resourceView = nullptr;
	_renderData.samplerState = nullptr;

	_renderData.mesh = nullptr;
}

DebugObject::~DebugObject()
{
	if (!_initialized)
		return;

	if (_renderData.samplerState != nullptr)
		_renderData.samplerState->Release();
	if (_renderData.resourceView != nullptr)
		_renderData.resourceView->Release();
	if (_renderData.texture2D != nullptr)
		_renderData.texture2D->Release();
	if (_renderData.lightingBuffer != nullptr)
		_renderData.lightingBuffer->Release();
	if (_renderData.matrixBuffer != nullptr)
		_renderData.matrixBuffer->Release();
	//if (_renderData.vertexBuffer != nullptr)
	//	_renderData.vertexBuffer->Release();
	if (_renderData.inputLayout != nullptr)
		_renderData.inputLayout->Release();
	if (_renderData.pShader != nullptr)
		_renderData.pShader->Release();
	if (_renderData.vShader != nullptr)
		_renderData.vShader->Release();
}


bool DebugObject::Initialize(ID3D11Device *device, MeshD3D11 *meshRef)
{
	if (_initialized)
		return false;

	if (meshRef == nullptr)
	{
		ErrMsg("meshRef is nullptr!");
		return false;
	}

	_renderData.mesh = meshRef;

	/*SimpleVertex mesh[] = {
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
	_renderData.vertexCount = sizeof(mesh) / sizeof(SimpleVertex);

	if (!SetupPipeline(device,
		_renderData.vertexBuffer, _renderData.matrixBuffer, _renderData.lightingBuffer,
		_renderData.texture2D, _renderData.resourceView, _renderData.samplerState,
		_renderData.vShader, _renderData.pShader, _renderData.inputLayout,
		mesh, _renderData.vertexCount))
		return false;*/

	if (!SetupPipeline(device,
		_renderData.matrixBuffer, _renderData.lightingBuffer,
		_renderData.texture2D, _renderData.resourceView, _renderData.samplerState,
		_renderData.vShader, _renderData.pShader, _renderData.inputLayout,
		nullptr, 0))
		return false;

	_initialized = true;
	return true;
}

bool DebugObject::Initialize(ID3D11Device *device, MeshD3D11 *meshRef, SimpleVertex *mesh, UINT vertexCount)
{
	/*if (_initialized)
		return false;

	_renderData.vertexCount = vertexCount;

	if (!SetupPipeline(device,
		_renderData.vertexBuffer, _renderData.matrixBuffer, _renderData.lightingBuffer,
		_renderData.texture2D, _renderData.resourceView, _renderData.samplerState,
		_renderData.vShader, _renderData.pShader, _renderData.inputLayout,
		mesh, vertexCount))
		return false;

	_initialized = true;
	return true;*/
	return false;
}

bool DebugObject::Uninitialize()
{
	if (!_initialized)
		return false;

	if (_renderData.samplerState != nullptr)
		_renderData.samplerState->Release();
	_renderData.samplerState = nullptr;

	if (_renderData.resourceView != nullptr)
		_renderData.resourceView->Release();
	_renderData.resourceView = nullptr;

	if (_renderData.texture2D != nullptr)
		_renderData.texture2D->Release();
	_renderData.texture2D = nullptr;

	if (_renderData.lightingBuffer != nullptr)
		_renderData.lightingBuffer->Release();
	_renderData.lightingBuffer = nullptr;

	if (_renderData.matrixBuffer != nullptr)
		_renderData.matrixBuffer->Release();
	_renderData.matrixBuffer = nullptr;

	/*if (_renderData.vertexBuffer != nullptr)
		_renderData.vertexBuffer->Release();
	_renderData.vertexBuffer = nullptr;*/

	if (_renderData.inputLayout != nullptr)
		_renderData.inputLayout->Release();
	_renderData.inputLayout = nullptr;

	if (_renderData.pShader != nullptr)
		_renderData.pShader->Release();
	_renderData.pShader = nullptr;

	if (_renderData.vShader != nullptr)
		_renderData.vShader->Release();
	_renderData.vShader = nullptr;

	_initialized = false;
	return true;
}


bool DebugObject::SetVPM(
	ID3D11DeviceContext *context, 
	const float fov, const float aspect, const float nearPlane, const float farPlane,
	const XMVECTOR &camPos, const XMVECTOR &camDir)
{
	if (!_initialized)
		return false;

	XMMATRIX viewProjMatrix =
		XMMatrixLookAtLH(camPos, camPos + camDir, { 0, 1, 0 }) *
		XMMatrixPerspectiveFovLH(fov * 0.0174533f, aspect, nearPlane, farPlane);

	XMStoreFloat4x4(&_renderData.matrixBufferData.viewProjMatrix, XMMatrixTranspose(viewProjMatrix));

	return true;
}

bool DebugObject::SetWM(ID3D11DeviceContext *context, const XMVECTOR &pos, const XMVECTOR &rot, const XMVECTOR &scale)
{
	if (!_initialized)
		return false;

	XMMATRIX worldMatrix =
		XMMatrixScalingFromVector(scale) *
		XMMatrixRotationRollPitchYawFromVector(rot) *
		XMMatrixTranslationFromVector(pos);

	XMStoreFloat4x4(&_renderData.matrixBufferData.worldMatrix, XMMatrixTranspose(worldMatrix));

	return true;
}


bool DebugObject::Update(ID3D11DeviceContext *context, const Time &time)
{
	if (!_initialized)
		return false;

	if (!UpdateMatrixBuffer(context, _renderData.matrixBuffer, _renderData.matrixBufferData))
		return false;

	if (!UpdateLightingBuffer(context, _renderData.lightingBuffer, _renderData.lightingBufferData))
		return false;

	return true;
}

bool DebugObject::Render(ID3D11DeviceContext *context)
{
	if (!_renderData.mesh->BindMeshBuffers(context))
	{
		ErrMsg("Failed to bind mesh buffer!");
		return false;
	}

	//constexpr UINT stride = sizeof(SimpleVertex);
	//constexpr UINT offset = 0;
	//context->IASetVertexBuffers(0, 1, &_renderData.vertexBuffer, &stride, &offset);
	context->IASetInputLayout(_renderData.inputLayout);
	context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	context->VSSetShader(_renderData.vShader, nullptr, 0);
	context->VSSetConstantBuffers(0, 1, &_renderData.matrixBuffer);

	context->PSSetShader(_renderData.pShader, nullptr, 0);
	context->PSSetConstantBuffers(0, 1, &_renderData.lightingBuffer);
	context->PSSetShaderResources(0, 1, &_renderData.resourceView);
	context->PSSetSamplers(0, 1, &_renderData.samplerState);

	//context->Draw(_renderData.vertexCount, 0);
	if (!_renderData.mesh->PerformSubMeshDrawCall(context, 0))
	{
		ErrMsg("Failed to perform sub-mesh draw call!");
		return false;
	}

	return true;
}
