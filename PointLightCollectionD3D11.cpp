#include "PointLightCollectionD3D11.h"

#include "ErrMsg.h"


PointLightCollectionD3D11::~PointLightCollectionD3D11()
{
	for (const CameraD3D11 *camera : _shadowCameras)
		delete camera;
}

bool PointLightCollectionD3D11::Initialize(ID3D11Device *device, const PointLightData &lightInfo)
{
	const UINT lightCount = static_cast<UINT>(lightInfo.perLightInfo.size());
	_bufferData.reserve(lightCount);

	for (UINT i = 0; i < lightCount; i++)
	{
		const PointLightData::PerLightInfo iLightInfo = lightInfo.perLightInfo.at(i);

		CameraD3D11 *lightCamera = new CameraD3D11(
			device,
			ProjectionInfo(90.0f, 1.0f, iLightInfo.projectionNearZ, iLightInfo.projectionFarZ),
			{ iLightInfo.initialPosition.x, iLightInfo.initialPosition.y, iLightInfo.initialPosition.z, 1.0f }
		);

		_shadowCameras.push_back(lightCamera);

		LightBuffer lightBuffer;
		lightBuffer.color = iLightInfo.color;
		lightBuffer.position = iLightInfo.initialPosition;
		lightBuffer.projectionNearZ = iLightInfo.projectionNearZ;
		lightBuffer.projectionFarZ = iLightInfo.projectionFarZ;

		_bufferData.push_back(lightBuffer);
	}

	if (!_shadowMaps.Initialize(device,
		lightInfo.shadowCubeMapInfo.textureDimension,
		lightInfo.shadowCubeMapInfo.textureDimension,
		true,
		lightCount))
	{
		ErrMsg("Failed to initialize point light shadow maps!");
		return false;
	}

	if (!_lightBuffer.Initialize(device,
		sizeof(LightBuffer),
		lightCount,
		_bufferData.data(),
		true))
	{
		ErrMsg("Failed to initialize point light buffer!");
		return false;
	}

	return true;
}

bool PointLightCollectionD3D11::UpdateLightBuffers(ID3D11DeviceContext *context) const
{
	const UINT lightCount = static_cast<UINT>(_bufferData.size());
	for (UINT i = 0; i < lightCount; i++)
	{
		const LightBuffer *lightBuffer = &_bufferData.at(i);

		if (!_lightBuffer.UpdateBuffer(context, lightBuffer))
		{
			ErrMsg(std::format("Failed to update point light buffer #{}!", i));
			return false;
		}
	}

	return true;
}


UINT PointLightCollectionD3D11::GetNrOfLights() const
{
	return static_cast<UINT>(_bufferData.size());
}

ID3D11DepthStencilView *PointLightCollectionD3D11::GetShadowMapDSV(const UINT lightIndex) const
{
	return _shadowMaps.GetDSV(lightIndex);
}

ID3D11ShaderResourceView *PointLightCollectionD3D11::GetShadowMapsSRV() const
{
	return _shadowMaps.GetSRV();
}

ID3D11ShaderResourceView *PointLightCollectionD3D11::GetLightBufferSRV() const
{
	return _lightBuffer.GetSRV();
}

CameraD3D11 *PointLightCollectionD3D11::GetLightCamera(const UINT lightIndex) const
{
	return _shadowCameras.at(lightIndex);
}

ID3D11Buffer *PointLightCollectionD3D11::GetLightCameraVSBuffer(const UINT lightIndex) const
{
	return _shadowCameras.at(lightIndex)->GetCameraVSBuffer();
}

ID3D11Buffer *PointLightCollectionD3D11::GetLightCameraCSBuffer(const UINT lightIndex) const
{
	return _shadowCameras.at(lightIndex)->GetCameraCSBuffer();
}
