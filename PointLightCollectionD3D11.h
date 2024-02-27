#pragma once

#include <vector>

#include <d3d11_4.h>
#include <DirectXMath.h>

#include "StructuredBufferD3D11.h"
#include "DepthBufferD3D11.h"
#include "CameraD3D11.h"

using namespace DirectX;


struct PointLightData
{
	struct ShadowMapInfo
	{
		UINT textureDimension = 0;
	} shadowCubeMapInfo;

	struct PerLightInfo
	{
		XMFLOAT3 color;
		XMFLOAT3 initialPosition;
		float projectionNearZ = 0.0f;
		float projectionFarZ = 0.0f;
	};

	std::vector<PerLightInfo> perLightInfo;
};


class PointLightCollectionD3D11
{
private:
	struct LightBuffer
	{
		XMFLOAT3 color = { };
		XMFLOAT3 position = { };
		float projectionNearZ = 0.0f;
		float projectionFarZ = 0.0f;
	};

	std::vector<LightBuffer> _bufferData;

	DepthBufferD3D11 _shadowMaps;
	StructuredBufferD3D11 _lightBuffer;
	std::vector<CameraD3D11 *> _shadowCameras;

public:
	PointLightCollectionD3D11() = default;
	~PointLightCollectionD3D11();
	PointLightCollectionD3D11(const PointLightCollectionD3D11 &other) = delete;
	PointLightCollectionD3D11 &operator=(const PointLightCollectionD3D11 &other) = delete;
	PointLightCollectionD3D11(PointLightCollectionD3D11 &&other) = delete;
	PointLightCollectionD3D11 &operator=(PointLightCollectionD3D11 &&other) = delete;

	[[nodiscard]] bool Initialize(ID3D11Device *device, const PointLightData &lightInfo);

	[[nodiscard]] bool UpdateLightBuffers(ID3D11DeviceContext *context) const;

	[[nodiscard]] UINT GetNrOfLights() const;
	[[nodiscard]] ID3D11DepthStencilView *GetShadowMapDSV(UINT lightIndex) const;
	[[nodiscard]] ID3D11ShaderResourceView *GetShadowMapsSRV() const;
	[[nodiscard]] ID3D11ShaderResourceView *GetLightBufferSRV() const;
	[[nodiscard]] CameraD3D11 *GetLightCamera(UINT lightIndex) const;
	[[nodiscard]] ID3D11Buffer *GetLightCameraVSBuffer(UINT lightIndex) const;
	[[nodiscard]] ID3D11Buffer *GetLightCameraCSBuffer(UINT lightIndex) const;
};