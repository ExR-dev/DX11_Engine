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
		XMFLOAT3 color;
		float rotationX = 0.0f;
		float rotationY = 0.0f;
		float angle = 0.0f;
		float projectionNearZ = 0.0f;
		float projectionFarZ = 0.0f;
		XMFLOAT3 initialPosition;
	};

	std::vector<PerLightInfo> perLightInfo;
};

class SpotLightCollectionD3D11
{
private:
	struct LightBuffer
	{
		XMFLOAT4X4 vpMatrix;
		XMFLOAT3 color;
		XMFLOAT3 direction;
		float angle = 0.0f;
		XMFLOAT3 position;
	};

	std::vector<LightBuffer> _bufferData;

	DepthBufferD3D11 _shadowMaps;
	StructuredBufferD3D11 _lightBuffer;
	std::vector<CameraD3D11*> _shadowCameras;

public:
	SpotLightCollectionD3D11() = default;
	~SpotLightCollectionD3D11();
	SpotLightCollectionD3D11(const SpotLightCollectionD3D11 &other) = delete;
	SpotLightCollectionD3D11 &operator=(const SpotLightCollectionD3D11 &other) = delete;
	SpotLightCollectionD3D11(SpotLightCollectionD3D11 &&other) = delete;
	SpotLightCollectionD3D11 &operator=(SpotLightCollectionD3D11 &&other) = delete;

	[[nodiscard]] bool Initialize(ID3D11Device *device, const SpotLightData &lightInfo);

	[[nodiscard]] bool UpdateLightBuffers(ID3D11DeviceContext *context) const;

	[[nodiscard]] UINT GetNrOfLights() const;
	[[nodiscard]] ID3D11DepthStencilView *GetShadowMapDSV(UINT lightIndex) const;
	[[nodiscard]] ID3D11ShaderResourceView *GetShadowMapsSRV() const;
	[[nodiscard]] ID3D11ShaderResourceView *GetLightBufferSRV() const;
	[[nodiscard]] CameraD3D11 *GetLightCamera(UINT lightIndex) const;
	[[nodiscard]] ID3D11Buffer *GetLightCameraVSBuffer(UINT lightIndex) const;
	[[nodiscard]] ID3D11Buffer *GetLightCameraCSBuffer(UINT lightIndex) const;
};