#pragma once

#include <d3d11_4.h>
#include <DirectXMath.h>

#include "ConstantBufferD3D11.h"
#include "PipelineHelper.h"
#include "Transform.h"


struct ProjectionInfo
{
	float fovAngleY = 80.0f;
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
	bool _isDirty = true;

	ConstantBufferD3D11 _lightingBuffer;
	LightingBufferData _lightingBufferData = {
		{0.0f, 0.0f, 0.0f, 1.0f}, // Camera position
		{0.5f, 2.0f, -2.5f, 1.0f}, // Light position

		{0.75f, 0.9f, 1.0f, 0.05f}, // Ambient
		{1.0f, 1.0f, 1.0f, 5.0f}, // Diffuse
		{10.0f, 10.0f, 10.0f, 128.0f}, // Specular
	};


	void Move(float amount, const XMFLOAT4A &direction);
	void MoveLocal(float amount, const XMFLOAT4A &direction);

public:
	CameraD3D11() = default;
	CameraD3D11(ID3D11Device *device, const ProjectionInfo &projectionInfo,
		const XMFLOAT4A &initialPosition = XMFLOAT4A(0.0f, 0.0f, 0.0f, 0.0f));
	~CameraD3D11() = default;
	CameraD3D11(const CameraD3D11 &other) = delete;
	CameraD3D11 &operator=(const CameraD3D11 &other) = delete;
	CameraD3D11(CameraD3D11 &&other) = delete;
	CameraD3D11 &operator=(CameraD3D11 &&other) = delete;

	bool Initialize(ID3D11Device *device, const ProjectionInfo &projectionInfo,
		const XMFLOAT4A &initialPosition = XMFLOAT4A(0.0f, 0.0f, 0.0f, 0.0f));

	void MoveForward(float amount);
	void MoveRight(float amount);
	void MoveUp(float amount);

	void RotateForward(float amount);
	void RotateRight(float amount);
	void RotateUp(float amount);

	void LookX(float amount);
	void LookY(float amount);

	[[nodiscard]] const XMFLOAT4A &GetPosition() const;
	[[nodiscard]] const XMFLOAT4A &GetForward() const;
	[[nodiscard]] const XMFLOAT4A &GetRight() const;
	[[nodiscard]] const XMFLOAT4A &GetUp() const;
	[[nodiscard]] const XMFLOAT4X4 &GetViewProjectionMatrix() const;

	bool UpdateBuffers(ID3D11DeviceContext *context);
	bool BindBuffers(ID3D11DeviceContext *context) const;

	[[nodiscard]] ID3D11Buffer *GetCameraBuffer() const;
	[[nodiscard]] ID3D11Buffer *GetLightingBuffer() const;
};