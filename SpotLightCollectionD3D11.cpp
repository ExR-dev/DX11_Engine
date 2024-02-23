#include "SpotLightCollectionD3D11.h"

#include "ErrMsg.h"


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
			lookDir;

		XMStoreFloat4x4(
			&lightBuffer.vpMatrix,
			XMMatrixTranspose(
				XMMatrixLookAtLH(
					pos,
					pos + lookDir,
					XMLoadFloat3({0, 1, 0})
				) *
				XMMatrixPerspectiveFovLH(
					iLightInfo.angle * (XM_PI / 180.0f),
					1.0f,
					iLightInfo.projectionNearZ,
					iLightInfo.projectionFarZ
				)
			)
		);

		_bufferData.push_back(lightBuffer);
	}

	return false;
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
	return _shadowCameras.at(lightIndex).GetCameraBuffer();
}
