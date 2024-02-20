#pragma once

#include <DirectXMath.h>

#include "ConstantBufferD3D11.h"

using namespace DirectX;


class Transform
{
private:
	XMFLOAT4A
		_right		= { 1.0f, 0.0f, 0.0f, 0.0f },
		_up			= { 0.0f, 1.0f, 0.0f, 0.0f },
		_forward	= { 0.0f, 0.0f, 1.0f, 0.0f },
		_pos		= { 0.0f, 0.0f, 0.0f, 1.0f };

	ConstantBufferD3D11 _worldMatrixBuffer;
	bool _isDirty = true;

public:
	Transform() = default;
	explicit Transform(ID3D11Device *device, const XMMATRIX &worldMatrix = XMMatrixIdentity());
	~Transform() = default;
	Transform(const Transform &other) = delete;
	Transform &operator=(const Transform &other) = delete;
	Transform(Transform &&other) = delete;
	Transform &operator=(Transform &&other) = delete;

	bool Initialize(ID3D11Device *device);
	bool Initialize(ID3D11Device *device, XMMATRIX worldMatrix);

	void Move(const XMFLOAT4A &movement);
	void Rotate(const XMFLOAT4A &rotation);
	void Scale(const XMFLOAT4A &scaling);

	void MoveLocal(const XMFLOAT4A &movement);
	void RotateLocal(const XMFLOAT4A &rotation);
	void ScaleLocal(const XMFLOAT4A &scaling);

	void RotateRoll(float rotation);
	void RotatePitch(float rotation);
	void RotateYaw(float rotation);

	void RotateByQuaternion(const XMVECTOR &quaternion);

	void SetPosition(const XMFLOAT4A &position);
	void SetAxes(const XMFLOAT4A &right, const XMFLOAT4A &up, const XMFLOAT4A &forward);
	void SetScale(const XMFLOAT4A &scale);

	[[nodiscard]] XMFLOAT4A GetPosition() const;
	[[nodiscard]] XMFLOAT4A GetRight() const;
	[[nodiscard]] XMFLOAT4A GetUp() const;
	[[nodiscard]] XMFLOAT4A GetForward() const;

	bool UpdateConstantBuffer(ID3D11DeviceContext *context);
	[[nodiscard]] ID3D11Buffer *GetConstantBuffer() const;
	[[nodiscard]] XMMATRIX GetWorldMatrix() const;
};
