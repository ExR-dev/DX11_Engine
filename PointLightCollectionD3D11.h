#pragma once

#include <vector>
#include <d3d11_4.h>
#include <DirectXMath.h>

#include "StructuredBufferD3D11.h"
#include "DepthBufferD3D11.h"
#include "CameraD3D11.h"


struct PointLightData
{
	struct ShadowMapInfo
	{
		UINT textureDimension = 0;
	} shadowCubeMapInfo;

	struct PerLightInfo
	{
		DirectX::XMFLOAT3 initialPosition;
		DirectX::XMFLOAT3 color;
		float falloff = 0.0f;
		float projectionNearZ = 0.0f;
		float projectionFarZ = 0.0f;
	};

	std::vector<PerLightInfo> perLightInfo;
};

struct ShadowCameraCube
{
	std::array<CameraD3D11 *, 6> cameraArray;
	uint8_t isEnabledFlag = 0b111111;
};


class PointLightCollectionD3D11
{
private:
	struct LightBuffer
	{
		DirectX::XMFLOAT4X4 vpMatrix = { };
		DirectX::XMFLOAT3 position = { };
		DirectX::XMFLOAT3 color = { };
		float falloff = 0.0f;
		float padding = 0.0f;
	};

	std::vector<LightBuffer> _bufferData;
	std::vector<ShadowCameraCube> _shadowCameraCubes;
	PointLightData::ShadowMapInfo _shadowMapInfo;

	DepthBufferD3D11 _shadowMaps;
	StructuredBufferD3D11 _lightBuffer;
	D3D11_VIEWPORT _shadowViewport = { };

public:
	PointLightCollectionD3D11() = default;
	~PointLightCollectionD3D11();
	PointLightCollectionD3D11(const PointLightCollectionD3D11 &other) = delete;
	PointLightCollectionD3D11 &operator=(const PointLightCollectionD3D11 &other) = delete;
	PointLightCollectionD3D11(PointLightCollectionD3D11 &&other) = delete;
	PointLightCollectionD3D11 &operator=(PointLightCollectionD3D11 &&other) = delete;

	[[nodiscard]] bool Initialize(ID3D11Device *device, const PointLightData &lightInfo);

	void Move(UINT lightIndex, DirectX::XMFLOAT4A movement);

	[[nodiscard]] bool UpdateBuffers(ID3D11DeviceContext *context);
	[[nodiscard]] bool BindCSBuffers(ID3D11DeviceContext *context) const;
	[[nodiscard]] bool BindPSBuffers(ID3D11DeviceContext *context) const;
	[[nodiscard]] bool UnbindCSBuffers(ID3D11DeviceContext *context) const;
	[[nodiscard]] bool UnbindPSBuffers(ID3D11DeviceContext *context) const;

	[[nodiscard]] UINT GetNrOfLights() const;
	[[nodiscard]] CameraD3D11 *GetLightCamera(UINT lightIndex, UINT cameraIndex) const;
	[[nodiscard]] ID3D11DepthStencilView *GetShadowMapDSV(UINT lightIndex, UINT cameraIndex) const;
	[[nodiscard]] ID3D11ShaderResourceView *GetShadowCubemapsSRV() const;
	[[nodiscard]] ID3D11ShaderResourceView *GetLightBufferSRV() const;
	[[nodiscard]] const D3D11_VIEWPORT &GetViewport() const;

	[[nodiscard]] bool IsEnabled(UINT lightIndex, UCHAR cameraIndex) const;
	void SetEnabled(UINT lightIndex, UCHAR cameraIndex, bool state);
};