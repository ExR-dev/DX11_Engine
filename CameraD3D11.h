#pragma once

#include <d3d11_4.h>
#include <DirectXMath.h>

#include "ConstantBufferD3D11.h"
#include "Transform.h"


struct ProjectionInfo
{
	float fovAngleY = 60.0f;
	float aspectRatio = 1.0f;
	float nearZ = 0.1f;
	float farZ = 50.0f;
};

class CameraD3D11
{
private:
	Transform _transform;
	ProjectionInfo _projInfo;

	ConstantBufferD3D11 _cameraBuffer;

	void MoveInDirection(float amount, const XMFLOAT3 &direction);
	void RotateAroundAxis(float amount, const XMFLOAT3 &axis);

public:
	CameraD3D11() = default;
	CameraD3D11(ID3D11Device *device, const ProjectionInfo &projectionInfo,
		const XMFLOAT3 &initialPosition = XMFLOAT3(0.0f, 0.0f, 0.0f));
	~CameraD3D11() = default;
	CameraD3D11(const CameraD3D11 &other) = delete;
	CameraD3D11 &operator=(const CameraD3D11 &other) = delete;
	CameraD3D11(CameraD3D11 &&other) = default;
	CameraD3D11 &operator=(CameraD3D11 &&other) = default;

	void Initialize(ID3D11Device *device, const ProjectionInfo &projectionInfo,
		const XMFLOAT3 &initialPosition = XMFLOAT3(0.0f, 0.0f, 0.0f));

	void MoveForward(float amount);
	void MoveRight(float amount);
	void MoveUp(float amount);

	void RotateForward(float amount);
	void RotateRight(float amount);
	void RotateUp(float amount);

	[[nodiscard]] const XMFLOAT3 &GetPosition() const;
	[[nodiscard]] const XMFLOAT3 &GetForward() const;
	[[nodiscard]] const XMFLOAT3 &GetRight() const;
	[[nodiscard]] const XMFLOAT3 &GetUp() const;

	void UpdateInternalConstantBuffer(ID3D11DeviceContext *context);
	[[nodiscard]] ID3D11Buffer *GetConstantBuffer() const;

	[[nodiscard]] XMFLOAT4X4 GetViewProjectionMatrix() const;
};