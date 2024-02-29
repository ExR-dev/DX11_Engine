#include "SpotLightCollectionD3D11.h"

#include "ErrMsg.h"


SpotLightCollectionD3D11::~SpotLightCollectionD3D11()
{
	for (const CameraD3D11 *camera : _shadowCameras)
		delete camera;
}

bool SpotLightCollectionD3D11::Initialize(ID3D11Device *device, const SpotLightData &lightInfo)
{
	const UINT lightCount = static_cast<UINT>(lightInfo.perLightInfo.size());
	_bufferData.reserve(lightCount);

	for (UINT i = 0; i < lightCount; i++)
	{
		// TODO: Very unsure if this implementation is correct

		const SpotLightData::PerLightInfo iLightInfo = lightInfo.perLightInfo.at(i);

		CameraD3D11 *lightCamera = new CameraD3D11(
			device,
			ProjectionInfo(iLightInfo.angle, 1.0f, iLightInfo.projectionNearZ, iLightInfo.projectionFarZ),
			{ iLightInfo.initialPosition.x, iLightInfo.initialPosition.y, iLightInfo.initialPosition.z, 1.0f },
			// TODO: false
			true
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

	_shadowViewport = { };
	_shadowViewport.TopLeftX = 0;
	_shadowViewport.TopLeftY = 0;
	_shadowViewport.Width = static_cast<float>(lightInfo.shadowMapInfo.textureDimension);
	_shadowViewport.Height = static_cast<float>(lightInfo.shadowMapInfo.textureDimension);
	_shadowViewport.MinDepth = 0;
	_shadowViewport.MaxDepth = 1;

	return true;
}


bool SpotLightCollectionD3D11::UpdateBuffers(ID3D11DeviceContext *context)
{
	const UINT lightCount = static_cast<UINT>(_bufferData.size());
	for (UINT i = 0; i < lightCount; i++)
	{
		CameraD3D11 &shadowCamera = *_shadowCameras.at(i);

		if (!shadowCamera.UpdateBuffers(context))
		{
			ErrMsg(std::format("Failed to update spotlight #{} camera buffers!", i));
			return false;
		}

		LightBuffer &lightBuffer = _bufferData.at(i);

		lightBuffer.vpMatrix = shadowCamera.GetViewProjectionMatrix();
		memcpy(&lightBuffer.position, &shadowCamera.GetPosition(), sizeof(XMFLOAT3));
		memcpy(&lightBuffer.direction, &shadowCamera.GetForward(), sizeof(XMFLOAT3));
	}

	if (!_lightBuffer.UpdateBuffer(context, _bufferData.data()))
	{
		ErrMsg("Failed to update light buffer!");
		return false;
	}

	return true;
}

bool SpotLightCollectionD3D11::BindBuffers(ID3D11DeviceContext *context) const
{
	ID3D11ShaderResourceView *const lightBufferSRV = _lightBuffer.GetSRV();
	context->CSSetShaderResources(3, 1, &lightBufferSRV);

	ID3D11ShaderResourceView *const shadowMapSRV = _shadowMaps.GetSRV();
	context->CSSetShaderResources(4, 1, &shadowMapSRV);

	return true;
}


UINT SpotLightCollectionD3D11::GetNrOfLights() const
{
	return static_cast<UINT>(_bufferData.size());
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

const D3D11_VIEWPORT &SpotLightCollectionD3D11::GetViewport() const
{
	return _shadowViewport;
}
