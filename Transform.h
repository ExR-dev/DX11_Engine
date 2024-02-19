#pragma once

#include <DirectXMath.h>

using namespace DirectX;


class Transform
{
private:
	XMFLOAT4A
		_right		= { 1.0f, 0.0f, 0.0f, 0.0f },
		_up			= { 0.0f, 1.0f, 0.0f, 0.0f },
		_forward	= { 0.0f, 0.0f, 1.0f, 0.0f },
		_pos		= { 0.0f, 0.0f, 0.0f, 1.0f };

public:
	void Move(const XMFLOAT4A &movement);
	void Rotate(const XMFLOAT4A &rotation);
	void Scale(const XMFLOAT4A &factor);

	void MoveLocal(const XMFLOAT4A &movement);
	void RotateLocal(const XMFLOAT4A &rotation);
	void ScaleLocal(const XMFLOAT4A &factor);

	void RotateRoll(const float &rotation);
	void RotatePitch(const float &rotation);
	void RotateYaw(const float &rotation);

	void RotateByQuaternion(const XMVECTOR &quaternion);

	void SetPosition(const XMFLOAT4A &position);
	void SetAxes(const XMFLOAT4A &right, const XMFLOAT4A &up, const XMFLOAT4A &forward);

	[[nodiscard]] XMFLOAT4A GetPosition() const;
	[[nodiscard]] XMFLOAT4A GetRight() const;
	[[nodiscard]] XMFLOAT4A GetUp() const;
	[[nodiscard]] XMFLOAT4A GetForward() const;
	[[nodiscard]] XMMATRIX GetWorldMatrix() const;
};
