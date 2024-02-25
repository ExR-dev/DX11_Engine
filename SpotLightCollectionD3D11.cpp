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
		const SpotLightData::PerLightInfo iLightInfo = lightInfo.perLightInfo.at(i);

		LightBuffer lightBuffer;
		lightBuffer.color = iLightInfo.color;
		lightBuffer.position = iLightInfo.initialPosition;
		lightBuffer.angle = iLightInfo.angle;

		XMVECTOR
			pos = XMLoadFloat3(&lightBuffer.position),
			rightDir = XMVectorSet(1, 0, 0, 0),
			upDir = XMVectorSet(0, 1, 0, 0),
			lookDir = XMVectorSet(0, 0, 1, 0);

		XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(0, iLightInfo.rotationX, 0);
		rightDir = XMVector3Transform(rightDir, rotationMatrix);
		lookDir = XMVector3Transform(lookDir, rotationMatrix);

		rotationMatrix = XMMatrixRotationNormal(rightDir, -iLightInfo.rotationY);
		upDir = XMVector3Normalize(XMVector3Transform(upDir, rotationMatrix));
		lookDir = XMVector3Normalize(XMVector3Transform(lookDir, rotationMatrix));

		XMStoreFloat4x4(
			&lightBuffer.vpMatrix,
			XMMatrixTranspose(
				XMMatrixLookAtLH(pos, pos + lookDir, upDir) *
				XMMatrixPerspectiveFovLH(
					iLightInfo.angle * (XM_PI / 180.0f),
					1.0f,
					iLightInfo.projectionNearZ,
					iLightInfo.projectionFarZ
				)
			)
		);

		XMStoreFloat3(&lightBuffer.direction, lookDir);

		_bufferData.push_back(lightBuffer);

		// TODO

		/*_shadowCameras.push_back(new CameraD3D11(
			device, 
			{ iLightInfo.angle, 1.0f, iLight. },
			//float fovAngleY = 80.0f;
			//float aspectRatio = 1.0f;
			//float nearZ = 0.1f;
			//float farZ = 50.0f;
		));

		_shadowCameras.back()*/
	}

	_shadowMaps.Initialize(device, lightInfo.shadowMapInfo.textureDimension, lightInfo.shadowMapInfo.textureDimension, true, lightCount);
	_lightBuffer.Initialize(device, sizeof(LightBuffer), lightCount, _bufferData.data(), true);

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

ID3D11Buffer *SpotLightCollectionD3D11::GetLightCameraConstantBuffer(const UINT lightIndex) const
{
	return _shadowCameras.at(lightIndex)->GetCameraBuffer();
}
