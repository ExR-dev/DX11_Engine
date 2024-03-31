#pragma once

#include <vector>

#include <d3d11_4.h>
#include <DirectXMath.h>

#include "StructuredBufferD3D11.h"
#include "DepthBufferD3D11.h"
#include "CameraD3D11.h"

using namespace DirectX;


struct SpotLightData
{
	struct ShadowMapInfo
	{
		UINT textureDimension = 0;
	} shadowMapInfo;

	struct PerLightInfo
	{
		XMFLOAT3 initialPosition = { };
		XMFLOAT3 color = { };
		float rotationX	= 0.0f;
		float rotationY	= 0.0f;
		float angle	= 0.0f;
		float falloff = 0.0f;
		float specularity = 0.0f;
		float projectionNearZ = 0.0f;
		float projectionFarZ = 0.0f;
	};

	std::vector<PerLightInfo> perLightInfo;
};

struct ShadowCamera
{
	CameraD3D11 *camera = nullptr;
	bool isEnabled = true;
};

class SpotLightCollectionD3D11
{
private:
	struct LightBuffer
	{
		XMFLOAT4X4 vpMatrix = { };
		XMFLOAT3 position = { };
		XMFLOAT3 direction = { };
		XMFLOAT3 color = { };
		float angle = 0.0f;
		float falloff = 0.0f;
		float specularity = 0.0f;
	};

	std::vector<LightBuffer> _bufferData;
	std::vector<ShadowCamera> _shadowCameras;
	SpotLightData::ShadowMapInfo _shadowMapInfo;

	DepthBufferD3D11 _shadowMaps;
	StructuredBufferD3D11 _lightBuffer;
	D3D11_VIEWPORT _shadowViewport = { };

	ID3D11RasterizerState *_rasterizerState = nullptr;

public:
	SpotLightCollectionD3D11() = default;
	~SpotLightCollectionD3D11();
	SpotLightCollectionD3D11(const SpotLightCollectionD3D11 &other) = delete;
	SpotLightCollectionD3D11 &operator=(const SpotLightCollectionD3D11 &other) = delete;
	SpotLightCollectionD3D11(SpotLightCollectionD3D11 &&other) = delete;
	SpotLightCollectionD3D11 &operator=(SpotLightCollectionD3D11 &&other) = delete;

	[[nodiscard]] bool Initialize(ID3D11Device *device, const SpotLightData &lightInfo);

	[[nodiscard]] bool ScaleLightFrustumsToCamera(const CameraD3D11 &viewCamera);
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
	[[nodiscard]] ID3D11RasterizerState *GetRasterizerState() const;
	[[nodiscard]] const D3D11_VIEWPORT &GetViewport() const;

	[[nodiscard]] bool IsEnabled(UINT lightIndex) const;
	void SetEnabled(UINT lightIndex, bool state);
};