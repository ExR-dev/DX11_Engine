#pragma once

#include <vector>

#include <d3d11_4.h>
#include <DirectXMath.h>

#include "StructuredBufferD3D11.h"
#include "DepthBufferD3D11.h"
#include "CameraD3D11.h"


struct DirLightData
{
	struct ShadowMapInfo
	{
		UINT textureDimension = 0;
	} shadowMapInfo;

	struct PerLightInfo
	{
		DirectX::XMFLOAT3 color = { };
		float rotationX	= 0.0f;
		float rotationY	= 0.0f;
	};

	std::vector<PerLightInfo> perLightInfo;
};

class DirLightCollectionD3D11
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
		DirectX::XMFLOAT3 direction = { };
		DirectX::XMFLOAT3 color = { };

		float padding[2];
	};

	std::vector<LightBuffer> _bufferData;
	std::vector<ShadowCamera> _shadowCameras;
	DirLightData::ShadowMapInfo _shadowMapInfo;

	DepthBufferD3D11 _shadowMaps;
	StructuredBufferD3D11 _lightBuffer;
	D3D11_VIEWPORT _shadowViewport = { };

public:
	DirLightCollectionD3D11() = default;
	~DirLightCollectionD3D11();
	DirLightCollectionD3D11(const DirLightCollectionD3D11 &other) = delete;
	DirLightCollectionD3D11 &operator=(const DirLightCollectionD3D11 &other) = delete;
	DirLightCollectionD3D11(DirLightCollectionD3D11 &&other) = delete;
	DirLightCollectionD3D11 &operator=(DirLightCollectionD3D11 &&other) = delete;

	[[nodiscard]] bool Initialize(ID3D11Device *device, const DirLightData &lightInfo);

	// Scale directional lights to the view, keeping the near-plane outside of the scene bounds.
	// If cubemap bounds are provided as well, the scene will be scaled to fit both.
	[[nodiscard]] bool ScaleToScene(CameraD3D11 &viewCamera, const DirectX::BoundingBox &sceneBounds, const DirectX::BoundingBox *cubemapBounds);

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

	void SetLightEnabled(UINT lightIndex, bool state);
	void SetLightColor(UINT lightIndex, const DirectX::XMFLOAT3 &color);
};