#pragma once

#include <array>
#include <d3d11_4.h>
#include <DirectXCollision.h>
#include <DirectXMath.h>
#include <vector>

#include "ConstantBufferD3D11.h"
#include "Transform.h"


struct ProjectionInfo
{
	float fovAngleY = 80.0f * (XM_PI / 180.0f);
	float aspectRatio = 1.0f;
	float nearZ = 0.1f;
	float farZ = 50.0f;
};

struct LightingBufferData
{
	float lightPos[4];
	float ambCol[4];
	float diffCol[4];
	float specCol[4];

	LightingBufferData(
		const std::array<float, 4> &lightPosition, 
		const std::array<float, 4> &ambientColour, 
		const std::array<float, 4> &diffuseColour, 
		const std::array<float, 4> &specularColour)
	{
		for (int i = 0; i < 4; ++i)
		{
			lightPos[i] = lightPosition[i];
			ambCol[i] = ambientColour[i];
			diffCol[i] = diffuseColour[i];
			specCol[i] = specularColour[i];
		}
	}
};

class CameraD3D11
{
private:
	Transform _transform;
	ProjectionInfo _defaultProjInfo, _currProjInfo;

	BoundingFrustum _frustum;

	ConstantBufferD3D11 _cameraVSBuffer;
	ConstantBufferD3D11 *_cameraCSBuffer = nullptr;
	bool _isDirty = true;

	void Move(float amount, const XMFLOAT4A &direction);
	void MoveLocal(float amount, const XMFLOAT4A &direction);

public:
	CameraD3D11() = default;
	CameraD3D11(ID3D11Device *device, const ProjectionInfo &projectionInfo,
		const XMFLOAT4A &initialPosition = XMFLOAT4A(0.0f, 0.0f, 0.0f, 0.0f), bool hasCSBuffer = true);
	~CameraD3D11();
	CameraD3D11(const CameraD3D11 &other) = delete;
	CameraD3D11 &operator=(const CameraD3D11 &other) = delete;
	CameraD3D11(CameraD3D11 &&other) = delete;
	CameraD3D11 &operator=(CameraD3D11 &&other) = delete;

	[[nodiscard]] bool Initialize(ID3D11Device *device, const ProjectionInfo &projectionInfo,
		const XMFLOAT4A &initialPosition = XMFLOAT4A(0.0f, 0.0f, 0.0f, 0.0f), bool hasCSBuffer = true);

	void MoveForward(float amount);
	void MoveRight(float amount);
	void MoveUp(float amount);

	void RotateRoll(float amount);
	void RotatePitch(float amount);
	void RotateYaw(float amount);

	void LookX(float amount);
	void LookY(float amount);

	[[nodiscard]] const XMFLOAT4A &GetPosition() const;
	[[nodiscard]] const XMFLOAT4A &GetForward() const;
	[[nodiscard]] const XMFLOAT4A &GetRight() const;
	[[nodiscard]] const XMFLOAT4A &GetUp() const;

	[[nodiscard]] XMFLOAT4X4A GetViewMatrix() const;
	[[nodiscard]] XMFLOAT4X4A GetProjectionMatrix() const;
	[[nodiscard]] XMFLOAT4X4A GetViewProjectionMatrix() const;

	[[nodiscard]] bool FitPlanesToPoints(const std::vector<XMFLOAT4A> &points);
	[[nodiscard]] bool UpdateBuffers(ID3D11DeviceContext *context);

	[[nodiscard]] bool BindGeometryBuffers(ID3D11DeviceContext *context) const;
	[[nodiscard]] bool BindLightingBuffers(ID3D11DeviceContext *context) const;

	[[nodiscard]] const Transform &GetTransform() const;
	[[nodiscard]] ID3D11Buffer *GetCameraVSBuffer() const;
	[[nodiscard]] ID3D11Buffer *GetCameraCSBuffer() const;
};