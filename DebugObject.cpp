#include "DebugObject.h"

#include "ErrMsg.h"


DebugObject::DebugObject()
{
	_initialized = false;

	_renderData.vShader = nullptr;
	_renderData.pShader = nullptr;
	_renderData.inputLayout = nullptr;
	_renderData.worldMatrixBuffer = nullptr;
	_renderData.viewProjMatrixBuffer = nullptr;
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
	if (_renderData.worldMatrixBuffer != nullptr)
		_renderData.worldMatrixBuffer->Release();
	if (_renderData.viewProjMatrixBuffer != nullptr)
		_renderData.viewProjMatrixBuffer->Release();
	if (_renderData.inputLayout != nullptr)
		_renderData.inputLayout->Release();
	if (_renderData.pShader != nullptr)
		_renderData.pShader->Release();
	if (_renderData.vShader != nullptr)
		_renderData.vShader->Release();
}


bool DebugObject::Initialize(ID3D11Device *device, const UINT meshID, const UINT vsID, const UINT psID)
{
	if (_initialized)
		return false;

	_renderData.meshID = meshID;
	_renderData.vsID = vsID;
	_renderData.psID = psID;

	if (!SetupPipeline(device,
		_renderData.worldMatrixBuffer, _renderData.viewProjMatrixBuffer, _renderData.lightingBuffer,
		_renderData.texture2D, _renderData.resourceView, _renderData.samplerState,
		_renderData.vShader, _renderData.pShader, _renderData.inputLayout,
		nullptr, 0))
		return false;

	_initialized = true;
	return true;
}


bool DebugObject::SetWM(ID3D11DeviceContext *context) const
{
	XMFLOAT4X4 worldMat;
	XMStoreFloat4x4(&worldMat, XMMatrixTranspose(_transform.GetWorldMatrix()));

	if (!_renderData.worldMatrixBuffer.UpdateBuffer(context, &worldMat))
	{
		ErrMsg("Failed to update world matrix buffer!");
		return false;
	}
	return true;
}


bool DebugObject::Update(ID3D11DeviceContext *context, const Time &time)
{
	if (!_initialized)
		return false;

	SetWM(context);

	// TODO: Deprecate
	if (!_renderData.lightingBuffer.UpdateBuffer(context, &_renderData.lightingBufferData))
	{
		ErrMsg("Failed to update lighting buffer!");
		return false;
	}

	return true;
}

bool DebugObject::Render(ID3D11DeviceContext *context)
{
	if (!_renderData.mesh->BindMeshBuffers(context))
	{
		ErrMsg("Failed to bind mesh buffers!");
		return false;
	}
	context->IASetInputLayout(_renderData.inputLayout);
	context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->VSSetShader(_renderData.vShader, nullptr, 0);
	context->VSSetConstantBuffers(0, 1, &_renderData.worldMatrixBuffer);
	context->VSSetConstantBuffers(1, 1, &_renderData.viewProjMatrixBuffer);

	context->PSSetShader(_renderData.pShader, nullptr, 0);
	context->PSSetConstantBuffers(0, 1, &_renderData.lightingBuffer);
	context->PSSetShaderResources(0, 1, &_renderData.resourceView);
	context->PSSetSamplers(0, 1, &_renderData.samplerState);

	const size_t subMeshCount = _renderData.mesh->GetNrOfSubMeshes();
	for (size_t i = 0; i < subMeshCount; i++)
	{
		if (!_renderData.mesh->PerformSubMeshDrawCall(context, i))
		{
			ErrMsg(std::format("Failed to perform sub-mesh draw call for submesh ID '{}'!", i));
			return false;
		}
	}

	return true;
}
