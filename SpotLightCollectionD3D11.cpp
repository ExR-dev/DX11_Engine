#include "SpotLightCollectionD3D11.h"

#include "ErrMsg.h"


SpotLightCollectionD3D11::~SpotLightCollectionD3D11()
{
	for (const CameraD3D11 *camera : _shadowCameras)
		delete camera;
}

bool SpotLightCollectionD3D11::Initialize(ID3D11Device *device, const SpotLightData &lightInfo)
{
	const UINT lightCount = lightInfo.perLightInfo.size();
	_bufferData.reserve(lightCount);

	for (UINT i = 0; i < lightCount; i++)
	{
		// TODO: Very unsure if this implementation is correct

		const SpotLightData::PerLightInfo iLightInfo = lightInfo.perLightInfo.at(i);

		CameraD3D11 *lightCamera = new CameraD3D11(
			device,
			ProjectionInfo(iLightInfo.angle, 1.0f, iLightInfo.projectionNearZ, iLightInfo.projectionFarZ),
			{ iLightInfo.initialPosition.x, iLightInfo.initialPosition.y, iLightInfo.initialPosition.z, 1.0f }
		);

		lightCamera->LookY(iLightInfo.rotationY);
		lightCamera->LookX(iLightInfo.rotationX);

		_shadowCameras.push_back(lightCamera);


		XMFLOAT4A dir = lightCamera->GetForward();

		LightBuffer lightBuffer;
		lightBuffer.vpMatrix = lightCamera->GetViewProjectionMatrix();
		lightBuffer.color = iLightInfo.color;
		lightBuffer.position = iLightInfo.initialPosition;
		lightBuffer.angle = iLightInfo.angle;
		lightBuffer.direction = { dir.x, dir.y, dir.z };

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

	if (!_lightBuffer.Initialize(device, 
		sizeof(LightBuffer), 
		lightCount, 
		_bufferData.data(), 
		true))
	{
		ErrMsg("Failed to initialize light buffer!");
		return false;
	}

	return true;
}

bool SpotLightCollectionD3D11::UpdateLightBuffers(ID3D11DeviceContext *context) const
{
	const UINT lightCount = _bufferData.size();
	for (UINT i = 0; i < lightCount; i++)
	{
		const LightBuffer *lightBuffer = &_bufferData.at(i);

		if (!_lightBuffer.UpdateBuffer(context, lightBuffer))
		{
			ErrMsg(std::format("Failed to update light buffer #{}!", i));
			return false;
		}
	}

	return true;
}


UINT SpotLightCollectionD3D11::GetNrOfLights() const
{
	return _bufferData.size();
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

CameraD3D11 *SpotLightCollectionD3D11::GetLightCamera(const UINT lightIndex) const
{
	return _shadowCameras.at(lightIndex);
}

ID3D11Buffer *SpotLightCollectionD3D11::GetLightCameraVSBuffer(const UINT lightIndex) const
{
	return _shadowCameras.at(lightIndex)->GetCameraVSBuffer();
}

ID3D11Buffer *SpotLightCollectionD3D11::GetLightCameraCSBuffer(const UINT lightIndex) const
{
	return _shadowCameras.at(lightIndex)->GetCameraCSBuffer();
}
