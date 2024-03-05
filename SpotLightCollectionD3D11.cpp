#include "SpotLightCollectionD3D11.h"

#include <DirectXCollision.h>

#include "ErrMsg.h"


SpotLightCollectionD3D11::~SpotLightCollectionD3D11()
{
	for (const ShadowCamera &shadowCamera : _shadowCameras)
	{
		if (shadowCamera.rasterizerState != nullptr)
			shadowCamera.rasterizerState->Release();

		delete shadowCamera.camera;
	}
}

bool SpotLightCollectionD3D11::Initialize(ID3D11Device *device, const SpotLightData &lightInfo)
{
	const UINT lightCount = static_cast<UINT>(lightInfo.perLightInfo.size());
	_bufferData.reserve(lightCount);

	for (UINT i = 0; i < lightCount; i++)
	{
		// TODO: Very unsure if this implementation is correct

		const SpotLightData::PerLightInfo iLightInfo = lightInfo.perLightInfo.at(i);

		_shadowCameras.push_back({ nullptr, nullptr, true });
		ShadowCamera &shadowCamera = _shadowCameras.back();


		D3D11_RASTERIZER_DESC rasterizerDesc = { };
		rasterizerDesc.FillMode = D3D11_FILL_SOLID;
		rasterizerDesc.CullMode = D3D11_CULL_BACK;
		rasterizerDesc.FrontCounterClockwise = false;
		rasterizerDesc.DepthBias = 1;
		rasterizerDesc.DepthBiasClamp = 0.0025f;
		rasterizerDesc.SlopeScaledDepthBias = 2.0f;
		rasterizerDesc.DepthClipEnable = false;
		rasterizerDesc.ScissorEnable = false;
		rasterizerDesc.MultisampleEnable = false;
		rasterizerDesc.AntialiasedLineEnable = false;

		if (FAILED(device->CreateRasterizerState(&rasterizerDesc, &shadowCamera.rasterizerState)))
		{
			ErrMsg(std::format("Failed to create rasterizer state for spotlight #{}!", i));
			return false;
		}


		shadowCamera.camera = new CameraD3D11(
			device,
			ProjectionInfo(iLightInfo.angle, 1.0f, iLightInfo.projectionNearZ, iLightInfo.projectionFarZ),
			{ iLightInfo.initialPosition.x, iLightInfo.initialPosition.y, iLightInfo.initialPosition.z, 1.0f },
			true // TODO: Make false, only true to control the camera for debugging purposes
		);

		shadowCamera.camera->LookY(iLightInfo.rotationY);
		shadowCamera.camera->LookX(iLightInfo.rotationX);


		XMFLOAT4A dir = shadowCamera.camera->GetForward();

		LightBuffer lightBuffer;
		lightBuffer.vpMatrix = shadowCamera.camera->GetViewProjectionMatrix();
		lightBuffer.color = iLightInfo.color;
		lightBuffer.position = iLightInfo.initialPosition;
		lightBuffer.angle = iLightInfo.angle;
		lightBuffer.falloff = iLightInfo.falloff;
		lightBuffer.specularity = iLightInfo.specularity;
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
	_shadowViewport.MinDepth = 0.0f;
	_shadowViewport.MaxDepth = 1.0f;

	return true;
}


bool SpotLightCollectionD3D11::ScaleLightFrustumsToCamera(const CameraD3D11 &viewCamera)
{
	const XMMATRIX cameraWorldMatrix = viewCamera.GetTransform().GetWorldMatrix();
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
		memcpy(&lightBuffer.position, &shadowCamera.camera->GetPosition(), sizeof(XMFLOAT3));
		memcpy(&lightBuffer.direction, &shadowCamera.camera->GetForward(), sizeof(XMFLOAT3));
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

ID3D11RasterizerState *SpotLightCollectionD3D11::GetLightRasterizer(const UINT lightIndex) const
{
	return _shadowCameras.at(lightIndex).rasterizerState;
}

CameraD3D11 *SpotLightCollectionD3D11::GetLightCamera(const UINT lightIndex) const
{
	return _shadowCameras.at(lightIndex).camera;
}

const D3D11_VIEWPORT &SpotLightCollectionD3D11::GetViewport() const
{
	return _shadowViewport;
}


bool SpotLightCollectionD3D11::IsEnabled(const UINT lightIndex) const
{
	return _shadowCameras.at(lightIndex).isEnabled;
}

void SpotLightCollectionD3D11::SetEnabled(const UINT lightIndex, const bool state)
{
	_shadowCameras.at(lightIndex).isEnabled = state;
}
