
#include "SpotLightCollectionD3D11.h"

#include <DirectXCollision.h>

#include "ErrMsg.h"

using namespace DirectX;


SpotLightCollectionD3D11::~SpotLightCollectionD3D11()
{
	for (const ShadowCamera &shadowCamera : _shadowCameras)
		delete shadowCamera.camera;
}

bool SpotLightCollectionD3D11::Initialize(ID3D11Device *device, const SpotLightData &lightInfo)
{
	const UINT lightCount = static_cast<UINT>(lightInfo.perLightInfo.size());
	_bufferData.reserve(lightCount);

	for (UINT i = 0; i < lightCount; i++)
	{
		const SpotLightData::PerLightInfo iLightInfo = lightInfo.perLightInfo.at(i);

		_shadowCameras.push_back({ nullptr, true });
		ShadowCamera &shadowCamera = _shadowCameras.back();

		shadowCamera.camera = new CameraD3D11(
			device,
			ProjectionInfo(iLightInfo.angle, 1.0f, iLightInfo.projectionNearZ, iLightInfo.projectionFarZ),
			{ iLightInfo.initialPosition.x, iLightInfo.initialPosition.y, iLightInfo.initialPosition.z, 1.0f },
			true, iLightInfo.orthographic
		);

		shadowCamera.camera->LookY(iLightInfo.rotationY);
		shadowCamera.camera->LookX(iLightInfo.rotationX);

		XMFLOAT4A dir = shadowCamera.camera->GetForward();

		LightBuffer lightBuffer;
		lightBuffer.vpMatrix = shadowCamera.camera->GetViewProjectionMatrix();
		lightBuffer.position = iLightInfo.initialPosition;
		lightBuffer.direction = { dir.x, dir.y, dir.z };
		lightBuffer.color = iLightInfo.color;
		lightBuffer.angle = iLightInfo.angle;
		lightBuffer.falloff = iLightInfo.falloff;
		lightBuffer.orthographic = iLightInfo.orthographic ? 1 : -1;

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
		ErrMsg("Failed to initialize spotlight buffer!");
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


bool SpotLightCollectionD3D11::ScaleLightFrustumsToCamera(CameraD3D11 &viewCamera)
{
	const XMMATRIX cameraWorldMatrix = XMLoadFloat4x4A(&viewCamera.GetTransform().GetWorldMatrix());
	XMFLOAT4X4A cameraProjectionMatrix = viewCamera.GetProjectionMatrix();

	BoundingFrustum viewFrustum;
	viewFrustum.CreateFromMatrix(viewFrustum, *reinterpret_cast<XMMATRIX *>(&cameraProjectionMatrix));
	viewFrustum.Transform(viewFrustum, cameraWorldMatrix);

	XMFLOAT3 corners[8];
	viewFrustum.GetCorners(corners);

	std::vector<XMFLOAT4A> frustumCorners;
	for (XMFLOAT3 &corner : corners)
		frustumCorners.push_back({ corner.x, corner.y, corner.z, 0.0f });

	for (ShadowCamera &shadowCamera : _shadowCameras)
		shadowCamera.isEnabled = shadowCamera.camera->FitPlanesToPoints(frustumCorners);

	return true;
}

bool SpotLightCollectionD3D11::UpdateBuffers(ID3D11DeviceContext *context)
{
	const UINT lightCount = static_cast<UINT>(_bufferData.size());
	for (UINT i = 0; i < lightCount; i++)
	{
		const ShadowCamera &shadowCamera = _shadowCameras.at(i);

		if (!shadowCamera.isEnabled)
			continue;

		if (!shadowCamera.camera->UpdateBuffers(context))
		{
			ErrMsg(std::format("Failed to update spotlight #{} camera buffers!", i));
			return false;
		}

		LightBuffer &lightBuffer = _bufferData.at(i);
		lightBuffer.vpMatrix = shadowCamera.camera->GetViewProjectionMatrix();

		XMFLOAT4A pos = shadowCamera.camera->GetPosition();
		XMFLOAT4A dir = shadowCamera.camera->GetForward();

		memcpy(&lightBuffer.position, &pos, sizeof(XMFLOAT3));
		memcpy(&lightBuffer.direction, &dir, sizeof(XMFLOAT3));
	}

	if (!_lightBuffer.UpdateBuffer(context, _bufferData.data()))
	{
		ErrMsg("Failed to update light buffer!");
		return false;
	}

	return true;
}

bool SpotLightCollectionD3D11::BindCSBuffers(ID3D11DeviceContext *context) const
{
	ID3D11ShaderResourceView *const lightBufferSRV = _lightBuffer.GetSRV();
	context->CSSetShaderResources(4, 1, &lightBufferSRV);

	ID3D11ShaderResourceView *const shadowMapSRV = _shadowMaps.GetSRV();
	context->CSSetShaderResources(5, 1, &shadowMapSRV);

	return true;
}

bool SpotLightCollectionD3D11::BindPSBuffers(ID3D11DeviceContext *context) const
{
	ID3D11ShaderResourceView *const lightBufferSRV = _lightBuffer.GetSRV();
	context->PSSetShaderResources(4, 1, &lightBufferSRV);

	ID3D11ShaderResourceView *const shadowMapSRV = _shadowMaps.GetSRV();
	context->PSSetShaderResources(5, 1, &shadowMapSRV);

	return true;
}

bool SpotLightCollectionD3D11::UnbindCSBuffers(ID3D11DeviceContext *context) const
{
	constexpr ID3D11ShaderResourceView *const nullSRV[2] = { nullptr, nullptr };
	context->CSSetShaderResources(4, 2, nullSRV);

	return true;
}

bool SpotLightCollectionD3D11::UnbindPSBuffers(ID3D11DeviceContext *context) const
{
	constexpr ID3D11ShaderResourceView *const nullSRV[2] = { nullptr, nullptr };
	context->PSSetShaderResources(4, 2, nullSRV);

	return true;
}


UINT SpotLightCollectionD3D11::GetNrOfLights() const
{
	return static_cast<UINT>(_bufferData.size());
}

CameraD3D11 *SpotLightCollectionD3D11::GetLightCamera(const UINT lightIndex) const
{
	return _shadowCameras.at(lightIndex).camera;
}

ID3D11DepthStencilView *SpotLightCollectionD3D11::GetShadowMapDSV(const UINT lightIndex) const
{
	return _shadowMaps.GetDSV(lightIndex);
}

ID3D11ShaderResourceView *SpotLightCollectionD3D11::GetShadowMapsSRV() const
{
	return _shadowMaps.GetSRV();
}

ID3D11ShaderResourceView *SpotLightCollectionD3D11::GetLightBufferSRV() const
{
	return _lightBuffer.GetSRV();
}

const D3D11_VIEWPORT &SpotLightCollectionD3D11::GetViewport() const
{
	return _shadowViewport;
}


bool SpotLightCollectionD3D11::GetLightEnabled(const UINT lightIndex) const
{
	return _shadowCameras.at(lightIndex).isEnabled;
}

const XMFLOAT3 &SpotLightCollectionD3D11::GetLightColor(const UINT lightIndex) const
{
	return _bufferData.at(lightIndex).color;
}

float SpotLightCollectionD3D11::GetLightAngle(const UINT lightIndex) const
{
	return _bufferData.at(lightIndex).angle;
}

float SpotLightCollectionD3D11::GetLightFalloff(const UINT lightIndex) const
{
	return _bufferData.at(lightIndex).falloff;
}

bool SpotLightCollectionD3D11::GetLightOrthographic(const UINT lightIndex) const
{
	return _bufferData.at(lightIndex).orthographic > 0;
}


void SpotLightCollectionD3D11::SetLightEnabled(const UINT lightIndex, const bool state)
{
	_shadowCameras.at(lightIndex).isEnabled = state;
}

void SpotLightCollectionD3D11::SetLightColor(const UINT lightIndex, const XMFLOAT3 &color)
{
	_bufferData.at(lightIndex).color = color;
}

void SpotLightCollectionD3D11::SetLightAngle(const UINT lightIndex, const float angle)
{
	_bufferData.at(lightIndex).angle = angle;
	_shadowCameras.at(lightIndex).camera->SetFOV(angle);
}

void SpotLightCollectionD3D11::SetLightFalloff(const UINT lightIndex, const float falloff)
{
	_bufferData.at(lightIndex).falloff = falloff;
}

void SpotLightCollectionD3D11::SetLightOrthographic(const UINT lightIndex, const bool state)
{
	_bufferData.at(lightIndex).orthographic = state ? 1 : -1;
	_shadowCameras.at(lightIndex).camera->SetOrtho(state);
}
