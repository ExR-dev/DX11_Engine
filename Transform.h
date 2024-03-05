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
		_pos		= { 0.0f, 0.0f, 0.0f, 1.0f },
		_scale		= { 1.0f, 1.0f, 1.0f, 0.0f };

	ConstantBufferD3D11 _worldMatrixBuffer;
	bool _isDirty = true;


	void NormalizeBases();
	void OrthogonalizeBases();

public:
	Transform() = default;
	explicit Transform(ID3D11Device *device, const XMMATRIX &worldMatrix = XMMatrixIdentity());
	~Transform() = default;
	Transform(const Transform &other) = delete;
	Transform &operator=(const Transform &other) = delete;
	Transform(Transform &&other) = delete;
	Transform &operator=(Transform &&other) = delete;

	[[nodiscard]] bool Initialize(ID3D11Device *device);
	[[nodiscard]] bool Initialize(ID3D11Device *device, const XMMATRIX& worldMatrix);

	void Move(const XMFLOAT4A &movement);
	void Rotate(const XMFLOAT4A &rotation);
	void ScaleAbsolute(const XMFLOAT4A &scale);

	void MoveLocal(const XMFLOAT4A &movement);
	void RotateLocal(const XMFLOAT4A &rotation);
	void ScaleRelative(const XMFLOAT4A &scale);

	void RotateByQuaternion(const XMVECTOR &quaternion);

	void SetPosition(const XMFLOAT4A &position);
	void SetScale(const XMFLOAT4A &scale);
	void SetAxes(const XMFLOAT4A &right, const XMFLOAT4A &up, const XMFLOAT4A &forward);

	[[nodiscard]] const XMFLOAT4A &GetPosition() const;
	[[nodiscard]] const XMFLOAT4A &GetScale() const;
	[[nodiscard]] const XMFLOAT4A &GetRight() const;
	[[nodiscard]] const XMFLOAT4A &GetUp() const;
	[[nodiscard]] const XMFLOAT4A &GetForward() const;

	[[nodiscard]] bool GetDirty() const;

	[[nodiscard]] bool UpdateConstantBuffer(ID3D11DeviceContext *context);
	[[nodiscard]] ID3D11Buffer *GetConstantBuffer() const;
	[[nodiscard]] XMMATRIX GetWorldMatrix() const;
};
