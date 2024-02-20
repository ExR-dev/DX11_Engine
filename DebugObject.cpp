#include "DebugObject.h"

#include "ErrMsg.h"


DebugObject::DebugObject()
{

}

DebugObject::~DebugObject()
{

}


bool DebugObject::Initialize(ID3D11Device *device, UINT inputLayoutID, const UINT meshID, const UINT vsID, const UINT psID, const UINT texID)
{
	if (_initialized)
		return false;

	_inputLayoutID = inputLayoutID;
	_meshID = meshID;
	_vsID = vsID;
	_psID = psID;
	_texID = texID;

	_initialized = true;
	return true;
}


bool DebugObject::Update(ID3D11DeviceContext *context, const Time &time)
{
	if (!_initialized)
		return false;

	if (!_transform.UpdateConstantBuffer(context))
	{
		ErrMsg("Failed to set world matrix buffer!");
		return false;
	}

	return true;
}

bool DebugObject::Render(ID3D11DeviceContext *context, const Graphics &graphics, const Content &content)
{
	const MeshD3D11 *mesh = content.GetMesh(_meshID);
	if (mesh == nullptr)
	{
		ErrMsg("Failed to get mesh!");
		return false;
	}

	if (!mesh->BindMeshBuffers(context))
	{
		ErrMsg("Failed to bind mesh buffers!");
		return false;
	}

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
