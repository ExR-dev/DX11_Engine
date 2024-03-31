#include "PointLightCollectionD3D11.h"

#include "ErrMsg.h"


PointLightCollectionD3D11::~PointLightCollectionD3D11()
{
	for (const ShadowCameraCube &cameraCube : _shadowCameraCubes)
		for (UINT i = 0; i < 6; i++)
			delete cameraCube.cameraArray[i];
}

bool PointLightCollectionD3D11::Initialize(ID3D11Device *device, const PointLightData &lightInfo)
{
	const UINT lightCount = static_cast<UINT>(lightInfo.perLightInfo.size());
	_bufferData.reserve(lightCount);

	for (UINT i = 0; i < lightCount; i++)
	{
		const PointLightData::PerLightInfo iLightInfo = lightInfo.perLightInfo.at(i);

		_shadowCameraCubes.push_back({ { }, 0b111111 });
		ShadowCameraCube &shadowCameraCube = _shadowCameraCubes.back();

		const ProjectionInfo projInfo{
			DirectX::XM_PIDIV2,
			1.0f,
			iLightInfo.projectionNearZ,
			iLightInfo.projectionFarZ
		};

		for (UINT j = 0; j < 6; j++)
			shadowCameraCube.cameraArray[j] = new CameraD3D11(
				device,
				projInfo,
				{ iLightInfo.initialPosition.x, iLightInfo.initialPosition.y, iLightInfo.initialPosition.z, 1.0f },
				false
			);

		// Orient the shadow cubemap cameras
		shadowCameraCube.cameraArray[0]->LookX(DirectX::XM_PIDIV2);
		shadowCameraCube.cameraArray[0]->RotateRoll(DirectX::XM_PI);
		shadowCameraCube.cameraArray[1]->LookX(-DirectX::XM_PIDIV2);
		shadowCameraCube.cameraArray[1]->RotateRoll(DirectX::XM_PI);
		shadowCameraCube.cameraArray[2]->LookY(DirectX::XM_PIDIV2);
		shadowCameraCube.cameraArray[3]->LookY(-DirectX::XM_PIDIV2);
		shadowCameraCube.cameraArray[4]->LookX(DirectX::XM_PI);
		shadowCameraCube.cameraArray[4]->RotateRoll(DirectX::XM_PI);
		shadowCameraCube.cameraArray[5]->RotateRoll(DirectX::XM_PI);

		for (UINT j = 0; j < 6; j++)
		{
			LightBuffer lightBuffer;
			lightBuffer.vpMatrix = shadowCameraCube.cameraArray[j]->GetViewProjectionMatrix();
			lightBuffer.position = iLightInfo.initialPosition;
			lightBuffer.color = iLightInfo.color;
			lightBuffer.falloff = iLightInfo.falloff;
			lightBuffer.specularity = iLightInfo.specularity;

			_bufferData.push_back(lightBuffer);
		}
	}

	if (!_shadowMaps.Initialize(device,
		lightInfo.shadowCubeMapInfo.textureDimension,
		lightInfo.shadowCubeMapInfo.textureDimension,
		true,
		lightCount * 6))
	{
		ErrMsg("Failed to initialize shadow maps!");
		return false;
	}

	if (!_lightBuffer.Initialize(device, sizeof(LightBuffer), lightCount * 6,
		true, false, true, _bufferData.data()))
	{
		ErrMsg("Failed to initialize pointlight buffer!");
		return false;
	}

	_shadowViewport = { };
	_shadowViewport.TopLeftX = 0;
	_shadowViewport.TopLeftY = 0;
	_shadowViewport.Width = static_cast<float>(lightInfo.shadowCubeMapInfo.textureDimension);
	_shadowViewport.Height = static_cast<float>(lightInfo.shadowCubeMapInfo.textureDimension);
	_shadowViewport.MinDepth = 0.0f;
	_shadowViewport.MaxDepth = 1.0f;

	return true;
}


void PointLightCollectionD3D11::Move(const UINT lightIndex, const DirectX::XMFLOAT4A movement)
{
	for (UINT i = 0; i < 6; i++)
	{
		_shadowCameraCubes.at(lightIndex).cameraArray[i]->Move(movement.x, { 1, 0, 0, 0 });
		_shadowCameraCubes.at(lightIndex).cameraArray[i]->Move(movement.y, { 0, 1, 0, 0 });
		_shadowCameraCubes.at(lightIndex).cameraArray[i]->Move(movement.z, { 0, 0, 1, 0 });
	}
}


bool PointLightCollectionD3D11::UpdateBuffers(ID3D11DeviceContext *context)
{
	const UINT lightCount = GetNrOfLights();
	for (UINT i = 0; i < lightCount; i++)
	{
		const ShadowCameraCube &shadowCameraCube = _shadowCameraCubes.at(i);

		for (UINT j = 0; j < 6; j++)
		{
			if (!IsEnabled(i, j))
				continue;

			if (!shadowCameraCube.cameraArray[j]->UpdateBuffers(context))
			{
				ErrMsg(std::format("Failed to update pointlight #{} camera #{} buffers!", i, j));
				return false;
			}

			LightBuffer &lightBuffer = _bufferData.at(i * 6 + j);
			lightBuffer.vpMatrix = shadowCameraCube.cameraArray[j]->GetViewProjectionMatrix();
			memcpy(&lightBuffer.position, &shadowCameraCube.cameraArray[0]->GetPosition(), sizeof(XMFLOAT3));
		}
	}

	if (!_lightBuffer.UpdateBuffer(context, _bufferData.data()))
	{
		ErrMsg("Failed to update light buffer!");
		return false;
	}

	return true;
}


bool PointLightCollectionD3D11::BindCSBuffers(ID3D11DeviceContext *context) const
{
	ID3D11ShaderResourceView *const lightBufferSRV = _lightBuffer.GetSRV();
	context->CSSetShaderResources(5, 1, &lightBufferSRV);

	ID3D11ShaderResourceView *const shadowMapSRV = _shadowMaps.GetSRV();
	context->CSSetShaderResources(6, 1, &shadowMapSRV);

	return true;
}

bool PointLightCollectionD3D11::BindPSBuffers(ID3D11DeviceContext *context) const
{
	ID3D11ShaderResourceView *const lightBufferSRV = _lightBuffer.GetSRV();
	context->PSSetShaderResources(5, 1, &lightBufferSRV);

	ID3D11ShaderResourceView *const shadowMapSRV = _shadowMaps.GetSRV();
	context->PSSetShaderResources(6, 1, &shadowMapSRV);

	return true;
}

bool PointLightCollectionD3D11::UnbindCSBuffers(ID3D11DeviceContext *context) const
{
	constexpr ID3D11ShaderResourceView *const nullSRV[2] = { nullptr, nullptr };
	context->CSSetShaderResources(5, 2, nullSRV);

	return true;
}

bool PointLightCollectionD3D11::UnbindPSBuffers(ID3D11DeviceContext *context) const
{
	constexpr ID3D11ShaderResourceView *const nullSRV[2] = { nullptr, nullptr };
	context->PSSetShaderResources(5, 2, nullSRV);

	return true;
}


UINT PointLightCollectionD3D11::GetNrOfLights() const
{
	return static_cast<UINT>(_shadowCameraCubes.size());
}

CameraD3D11 *PointLightCollectionD3D11::GetLightCamera(const UINT lightIndex, const UINT cameraIndex) const
{
	return _shadowCameraCubes.at(lightIndex).cameraArray[cameraIndex];
}

ID3D11DepthStencilView *PointLightCollectionD3D11::GetShadowMapDSV(const UINT lightIndex, const UINT cameraIndex) const
{
	return _shadowMaps.GetDSV(lightIndex * 6 + cameraIndex);
}

ID3D11ShaderResourceView *PointLightCollectionD3D11::GetShadowCubemapsSRV() const
{
	return _shadowMaps.GetSRV();
}

ID3D11ShaderResourceView *PointLightCollectionD3D11::GetLightBufferSRV() const
{
	return _lightBuffer.GetSRV();
}

const D3D11_VIEWPORT &PointLightCollectionD3D11::GetViewport() const
{
	return _shadowViewport;
}


bool PointLightCollectionD3D11::IsEnabled(const UINT lightIndex, const UCHAR cameraIndex) const
{
	return (_shadowCameraCubes.at(lightIndex).isEnabledFlag & ((UCHAR)0b000001 << cameraIndex)) > 0;
}

void PointLightCollectionD3D11::SetEnabled(const UINT lightIndex, const UCHAR cameraIndex, const bool state)
{
	if (IsEnabled(lightIndex, cameraIndex) == state) return;
	_shadowCameraCubes.at(lightIndex).isEnabledFlag += (state ? 1 : -1) * ((UCHAR)0b000001 << cameraIndex);
}

