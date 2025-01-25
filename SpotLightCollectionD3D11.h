#pragma once

#include <vector>

#include <d3d11_4.h>
#include <DirectXMath.h>

#include "StructuredBufferD3D11.h"
#include "DepthBufferD3D11.h"
#include "CameraD3D11.h"


struct SpotLightData
{
	struct ShadowMapInfo
	{
		UINT textureDimension = 0;
	} shadowMapInfo;

	struct PerLightInfo
	{
		DirectX::XMFLOAT3 initialPosition = { };
		DirectX::XMFLOAT3 color = { };
		float rotationX	= 0.0f;
		float rotationY	= 0.0f;
		float angle	= 0.0f;
		float falloff = 0.0f;
		bool orthographic = false;
		float projectionNearZ = 0.0f;
		float projectionFarZ = 0.0f;
	};

	std::vector<PerLightInfo> perLightInfo;
};

class SpotLightCollectionD3D11
{
private:
	struct ShadowCamera
	{
		CameraD3D11 *camera = nullptr;
		bool isEnabled = true;
	};

	struct LightBuffer
	{
		DirectX::XMFLOAT4X4 vpMatrix = { };
		DirectX::XMFLOAT3 position = { };
		DirectX::XMFLOAT3 direction = { };
		DirectX::XMFLOAT3 color = { };
		float angle = 0.0f;
		float falloff = 0.0f;
		int orthographic = -1;
	};

	std::vector<LightBuffer> _bufferData;
	std::vector<ShadowCamera> _shadowCameras;
	SpotLightData::ShadowMapInfo _shadowMapInfo;

	DepthBufferD3D11 _shadowMaps;
	StructuredBufferD3D11 _lightBuffer;
	D3D11_VIEWPORT _shadowViewport = { };

public:
	SpotLightCollectionD3D11() = default;
	~SpotLightCollectionD3D11();
	SpotLightCollectionD3D11(const SpotLightCollectionD3D11 &other) = delete;
	SpotLightCollectionD3D11 &operator=(const SpotLightCollectionD3D11 &other) = delete;
	SpotLightCollectionD3D11(SpotLightCollectionD3D11 &&other) = delete;
	SpotLightCollectionD3D11 &operator=(SpotLightCollectionD3D11 &&other) = delete;

	[[nodiscard]] bool Initialize(ID3D11Device *device, const SpotLightData &lightInfo);

	[[nodiscard]] bool ScaleLightFrustumsToCamera(CameraD3D11 &viewCamera);
	[[nodiscard]] bool UpdateBuffers(ID3D11DeviceContext *context);
	[[nodiscard]] bool BindCSBuffers(ID3D11DeviceContext *context) const;
	[[nodiscard]] bool BindPSBuffers(ID3D11DeviceContext *context) const;
	[[nodiscard]] bool UnbindCSBuffers(ID3D11DeviceContext *context) const;
	[[nodiscard]] bool UnbindPSBuffers(ID3D11DeviceContext *context) const;

	[[nodiscard]] UINT GetNrOfLights() const;
	[[nodiscard]] CameraD3D11 *GetLightCamera(UINT lightIndex) const;
	[[nodiscard]] ID3D11DepthStencilView *GetShadowMapDSV(UINT lightIndex) const;
	[[nodiscard]] ID3D11ShaderResourceView *GetShadowMapsSRV() const;
	[[nodiscard]] ID3D11ShaderResourceView *GetLightBufferSRV() const;
	[[nodiscard]] const D3D11_VIEWPORT &GetViewport() const;


	[[nodiscard]] bool GetLightEnabled(UINT lightIndex) const;
	[[nodiscard]] const DirectX::XMFLOAT3 &GetLightColor(UINT lightIndex) const;
	[[nodiscard]] float GetLightAngle(UINT lightIndex) const;
	[[nodiscard]] float GetLightFalloff(UINT lightIndex) const;
	[[nodiscard]] bool GetLightOrthographic(UINT lightIndex) const;

	void SetLightEnabled(UINT lightIndex, bool state);
	void SetLightColor(UINT lightIndex, const DirectX::XMFLOAT3 &color);
	void SetLightAngle(UINT lightIndex, float angle);
	void SetLightFalloff(UINT lightIndex, float falloff);
	void SetLightOrthographic(UINT lightIndex, bool state);
};