#pragma once

#include <DirectXMath.h>

using namespace DirectX;


class Transform
{
private:

	XMFLOAT3
		_pos	= { 0.0f, 0.0f, 0.0f },
		_rot	= { 0.0f, 0.0f, 0.0f },
		_scale	= { 1.0f, 1.0f, 1.0f };

	XMMATRIX _world = XMMatrixIdentity();
	bool _dirty = true;

	void UpdateMatrix();

public:
	void Move(const XMFLOAT3 &movement);
	void Rotate(const XMFLOAT3 &rotation);
	void Scale(const XMFLOAT3 &factor);

	void AxisRotation(const XMFLOAT3 &axis, float angle);

	void MoveLocal(const XMFLOAT3 &movement);

	void SetPosition(const XMFLOAT3 &position);
	void SetRotation(const XMFLOAT3 &rotation);
	void SetScale(const XMFLOAT3 &scale);

	void SetWorldMatrix(const XMMATRIX &matrix);

	[[nodiscard]] XMFLOAT3 GetPosition() const;
	[[nodiscard]] XMFLOAT3 GetRotation() const;
	[[nodiscard]] XMFLOAT3 GetScale() const;

	[[nodiscard]] XMFLOAT3 GetRight();
	[[nodiscard]] XMFLOAT3 GetUp();
	[[nodiscard]] XMFLOAT3 GetForward();

	[[nodiscard]] XMMATRIX GetWorldMatrix();
};
