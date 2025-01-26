
#include "DirLightCollectionD3D11.h"

#include <DirectXCollision.h>

#include "ErrMsg.h"

using namespace DirectX;


DirLightCollectionD3D11::~DirLightCollectionD3D11()
{
	for (const ShadowCamera &shadowCamera : _shadowCameras)
		delete shadowCamera.camera;
}

bool DirLightCollectionD3D11::Initialize(ID3D11Device *device, const DirLightData &lightInfo)
{
	const UINT lightCount = static_cast<UINT>(lightInfo.perLightInfo.size());
	_bufferData.reserve(lightCount);

	for (UINT i = 0; i < lightCount; i++)
	{
		const DirLightData::PerLightInfo iLightInfo = lightInfo.perLightInfo.at(i);

		_shadowCameras.push_back({ nullptr, true });
		ShadowCamera &shadowCamera = _shadowCameras.back();

		shadowCamera.camera = new CameraD3D11(
			device,
			ProjectionInfo(1.0f, 1.0f, 0.1f, 1.0f),
			{ 0.0f, 0.0f, 0.0f, 1.0f },
			true, true
		);

		shadowCamera.camera->LookY(iLightInfo.rotationY);
		shadowCamera.camera->LookX(iLightInfo.rotationX);

		XMFLOAT4A dir = shadowCamera.camera->GetForward();

		LightBuffer lightBuffer;
		lightBuffer.vpMatrix = shadowCamera.camera->GetViewProjectionMatrix();
		lightBuffer.direction = { dir.x, dir.y, dir.z };
		lightBuffer.color = iLightInfo.color;

		_bufferData.push_back(lightBuffer);
	}

	if (!_shadowMaps.Initialize(device, 
		lightInfo.shadowMapInfo.textureDimension, 
		lightInfo.shadowMapInfo.textureDimension, 
		true, 
		lightCount))
	{
		ErrMsg("Failed to initialize shadow maps!");
		return false;
	}

	if (!_lightBuffer.Initialize(device, sizeof(LightBuffer), lightCount,
		true, false, true, _bufferData.data()))
	{
		ErrMsg("Failed to initialize directional light buffer!");
		return false;
	}

	_shadowViewport = { };
	_shadowViewport.TopLeftX = 0;
	_shadowViewport.TopLeftY = 0;
	_shadowViewport.Width = static_cast<float>(lightInfo.shadowMapInfo.textureDimension);
	_shadowViewport.Height = static_cast<float>(lightInfo.shadowMapInfo.textureDimension);
	_shadowViewport.MinDepth = 0.0f;
	_shadowViewport.MaxDepth = 1.0f;

	return true;
}


bool DirLightCollectionD3D11::ScaleToScene(CameraD3D11 &viewCamera, const BoundingBox &sceneBounds, const BoundingBox *cubemapBounds)
{
	XMFLOAT3 sceneCorners[8];
	sceneBounds.GetCorners(sceneCorners);

	std::vector<XMFLOAT4A> nearCorners;
	for (XMFLOAT3 &corner : sceneCorners)
		nearCorners.push_back({ corner.x, corner.y, corner.z, 1.0f });

	XMFLOAT3 cameraCorners[8];
	if (viewCamera.GetOrtho())
	{
		BoundingOrientedBox viewOrientedBox;
		if (!viewCamera.StoreBounds(viewOrientedBox))
		{
			ErrMsg("Failed to store camera oriented box!");
			return false;
		}
		viewOrientedBox.GetCorners(cameraCorners);
	}
	else
	{
		BoundingFrustum viewFrustum;
		if (!viewCamera.StoreBounds(viewFrustum))
		{
			ErrMsg("Failed to store camera frustum!");
			return false;
		}
		viewFrustum.GetCorners(cameraCorners);
	}

	std::vector<XMFLOAT4A> innerCorners;
	for (XMFLOAT3 &corner : cameraCorners)
		innerCorners.push_back({ corner.x, corner.y, corner.z, 1.0f });

	if (cubemapBounds != nullptr)
	{
		XMFLOAT3 cubemapCorners[8];
		cubemapBounds->GetCorners(cubemapCorners);

		for (XMFLOAT3 &corner : cubemapCorners)
			innerCorners.push_back({ corner.x, corner.y, corner.z, 1.0f });
	}

	for (ShadowCamera &shadowCamera : _shadowCameras)
		shadowCamera.isEnabled = shadowCamera.camera->ScaleToContents(nearCorners, innerCorners);

	return true;
}

bool DirLightCollectionD3D11::UpdateBuffers(ID3D11DeviceContext *context)
{
	const UINT lightCount = static_cast<UINT>(_bufferData.size());
	for (UINT i = 0; i < lightCount; i++)
	{
		const ShadowCamera &shadowCamera = _shadowCameras.at(i);

		if (!shadowCamera.isEnabled)
			continue;

		if (!shadowCamera.camera->UpdateBuffers(context))
		{
			ErrMsg(std::format("Failed to update dirlight #{} camera buffers!", i));
			return false;
		}

		LightBuffer &lightBuffer = _bufferData.at(i);
		lightBuffer.vpMatrix = shadowCamera.camera->GetViewProjectionMatrix();
		XMFLOAT4A dir = shadowCamera.camera->GetForward();
		memcpy(&lightBuffer.direction, &dir, sizeof(XMFLOAT3));
	}

	if (!_lightBuffer.UpdateBuffer(context, _bufferData.data()))
	{
		ErrMsg("Failed to update light buffer!");
		return false;
	}

	return true;
}

bool DirLightCollectionD3D11::BindCSBuffers(ID3D11DeviceContext *context) const
{
	ID3D11ShaderResourceView *const lightBufferSRV = _lightBuffer.GetSRV();
	context->CSSetShaderResources(8, 1, &lightBufferSRV);

	ID3D11ShaderResourceView *const shadowMapSRV = _shadowMaps.GetSRV();
	context->CSSetShaderResources(9, 1, &shadowMapSRV);

	return true;
}

bool DirLightCollectionD3D11::BindPSBuffers(ID3D11DeviceContext *context) const
{
	ID3D11ShaderResourceView *const lightBufferSRV = _lightBuffer.GetSRV();
	context->PSSetShaderResources(8, 1, &lightBufferSRV);

	ID3D11ShaderResourceView *const shadowMapSRV = _shadowMaps.GetSRV();
	context->PSSetShaderResources(9, 1, &shadowMapSRV);

	return true;
}

bool DirLightCollectionD3D11::UnbindCSBuffers(ID3D11DeviceContext *context) const
{
	constexpr ID3D11ShaderResourceView *const nullSRV[2] = { nullptr, nullptr };
	context->CSSetShaderResources(8, 2, nullSRV);

	return true;
}

bool DirLightCollectionD3D11::UnbindPSBuffers(ID3D11DeviceContext *context) const
{
	constexpr ID3D11ShaderResourceView *const nullSRV[2] = { nullptr, nullptr };
	context->PSSetShaderResources(8, 2, nullSRV);

	return true;
}


UINT DirLightCollectionD3D11::GetNrOfLights() const
{
	return static_cast<UINT>(_bufferData.size());
}

CameraD3D11 *DirLightCollectionD3D11::GetLightCamera(const UINT lightIndex) const
{
	return _shadowCameras.at(lightIndex).camera;
}

ID3D11DepthStencilView *DirLightCollectionD3D11::GetShadowMapDSV(const UINT lightIndex) const
{
	return _shadowMaps.GetDSV(lightIndex);
}

ID3D11ShaderResourceView *DirLightCollectionD3D11::GetShadowMapsSRV() const
{
	return _shadowMaps.GetSRV();
}

ID3D11ShaderResourceView *DirLightCollectionD3D11::GetLightBufferSRV() const
{
	return _lightBuffer.GetSRV();
}

const D3D11_VIEWPORT &DirLightCollectionD3D11::GetViewport() const
{
	return _shadowViewport;
}


bool DirLightCollectionD3D11::GetLightEnabled(const UINT lightIndex) const
{
	return _shadowCameras.at(lightIndex).isEnabled;
}

const XMFLOAT3 &DirLightCollectionD3D11::GetLightColor(const UINT lightIndex) const
{
	return _bufferData.at(lightIndex).color;
}


void DirLightCollectionD3D11::SetLightEnabled(const UINT lightIndex, const bool state)
{
	_shadowCameras.at(lightIndex).isEnabled = state;
}

void DirLightCollectionD3D11::SetLightColor(const UINT lightIndex, const XMFLOAT3 &color)
{
	_bufferData.at(lightIndex).color = color;
}
