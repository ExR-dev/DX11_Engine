#include "Transform.h"

#include "ErrMsg.h"


void Transform::Move(const XMFLOAT3 &movement)
{
	_pos.x += movement.x;
	_pos.y += movement.y;
	_pos.z += movement.z;
}

void Transform::Rotate(const XMFLOAT3 &rotation)
{
	_rot.x += rotation.x;
	_rot.y += rotation.y;
	_rot.z += rotation.z;

	_rot.x = fmodf(_rot.x, XM_2PI);
	if (_rot.x < 0.0f) _rot.x += XM_2PI;

	_rot.y = fmodf(_rot.y, XM_2PI);
	if (_rot.y < 0.0f) _rot.y += XM_2PI;

	_rot.z = fmodf(_rot.z, XM_2PI);
	if (_rot.z < 0.0f) _rot.z += XM_2PI;
}

void Transform::Scale(const XMFLOAT3 &factor)
{
	_right.x *= factor.x;
	_right.y *= factor.y;
	_right.z *= factor.z;
}


void Transform::RotateByQuaternion(const XMVECTOR &quaternion)
{
	const XMVECTOR quatConjugate = XMQuaternionConjugate(quaternion);

	const XMVECTOR
		newRight = quaternion * XMLoadFloat3(&_right)	* quatConjugate,
		newUp = quaternion * XMLoadFloat3(&_up)		* quatConjugate,
		newForward = quaternion * XMLoadFloat3(&_forward)	* quatConjugate;

	_right = { newRight.m128_f32[0],	 newRight.m128_f32[1],	 newRight.m128_f32[2] };
	_up = { newUp.m128_f32[0],		 newUp.m128_f32[1],		 newUp.m128_f32[2] };
	_forward = { newForward.m128_f32[0], newForward.m128_f32[1], newForward.m128_f32[2] };
}


XMMATRIX Transform::GetWorldMatrix() const
{
	const XMMATRIX worldMatrix = XMMatrixSet(
		_right.x, _right.y, _right.z, 0,
		_up.x, _up.y, _up.z, 0,
		_forward.x, _forward.y, _forward.z, 0,
		_pos.x, _pos.y, _pos.z, 1
	);

	return worldMatrix;
}







/*void Transform::AxisRotation(const XMFLOAT3 &axis, const float angle)
{
	// TODO: This is incorrect, decomposition gives rotation in quaternion form, not euler angles
	ErrMsg("Transform::AxisRotation() is incorrect, decomposition gives rotation in quaternion form, not euler angles");

	_wMat =
		XMMatrixScaling(_scale.x, _scale.y, _scale.z) *
		XMMatrixRotationRollPitchYaw(_rot.x, _rot.y, _rot.z) *
		XMMatrixRotationAxis(XMLoadFloat3(&axis), angle) *
		XMMatrixTranslation(_pos.x, _pos.y, _pos.z);

	XMVECTOR scale, rot, pos;
	XMMatrixDecompose(&scale, &rot, &pos, _wMat);

	_rot.x = rot.m128_f32[0];
	_rot.y = rot.m128_f32[1];
	_rot.z = rot.m128_f32[2];

	_wMatDirty = false;
}


void Transform::MoveLocal(const XMFLOAT3 &movement)
{
	const XMFLOAT3
		right = GetRight(),
		fwd = GetForward(),
		up = GetUp();

	const XMFLOAT3 localMovement = {
		(right.x * movement.x) + (up.x * movement.y) + (fwd.x * movement.z),
		(right.y * movement.x) + (up.y * movement.y) + (fwd.y * movement.z),
		(right.z * movement.x) + (up.z * movement.y) + (fwd.z * movement.z)
	};

	Move(localMovement);
}


void Transform::SetPosition(const XMFLOAT3 &position)
{
	_pos = position;
	_wMatDirty = true;
}

void Transform::SetRotation(const XMFLOAT3 &rotation)
{
	_rot = rotation;

	_rot.x = fmodf(_rot.x, XM_2PI);
	if (_rot.x < 0.0f) _rot.x += XM_2PI;

	_rot.y = fmodf(_rot.y, XM_2PI);
	if (_rot.y < 0.0f) _rot.y += XM_2PI;

	_rot.z = fmodf(_rot.z, XM_2PI);
	if (_rot.z < 0.0f) _rot.z += XM_2PI;

	_wMatDirty = true;
}

void Transform::SetScale(const XMFLOAT3 &scale)
{
	_scale = scale;
	_wMatDirty = true;
}


void Transform::SetWorldMatrix(const XMMATRIX &matrix)
{
	// TODO: This is incorrect, decomposition gives rotation in quaternion form, not euler angles
	ErrMsg("Transform::SetWorldMatrix() is incorrect, decomposition gives rotation in quaternion form, not euler angles");

	XMVECTOR scale, rot, pos;
	XMMatrixDecompose(&scale, &rot, &pos, _wMat);

	_scale.x = scale.m128_f32[0];
	_scale.y = scale.m128_f32[1];
	_scale.z = scale.m128_f32[2];

	_rot.x = rot.m128_f32[0];
	_rot.y = rot.m128_f32[1];
	_rot.z = rot.m128_f32[2];

	_pos.x = pos.m128_f32[0];
	_pos.y = pos.m128_f32[1];
	_pos.z = pos.m128_f32[2];

	_wMat = matrix;
	_wMatDirty = false;
}


XMFLOAT3 Transform::GetPosition() const
{
	return _pos;
}

XMFLOAT3 Transform::GetRotation() const
{
	return _rot;
}

XMFLOAT3 Transform::GetScale() const
{
	return _scale;
}


XMFLOAT3 Transform::GetRight()
{
	if (_wMatDirty) UpdateMatrix();

	XMVECTOR right = {
		_wMat.r[0].m128_f32[0],
		_wMat.r[0].m128_f32[1],
		_wMat.r[0].m128_f32[2],
		0.0f
	};

	right = XMVector3Normalize(right);

	return { right.m128_f32[0], right.m128_f32[1], right.m128_f32[2] };
}

XMFLOAT3 Transform::GetUp()
{
	if (_wMatDirty) UpdateMatrix();

	XMVECTOR up = {
		_wMat.r[1].m128_f32[0],
		_wMat.r[1].m128_f32[1],
		_wMat.r[1].m128_f32[2],
		0.0f
	};

	up = XMVector3Normalize(up);

	return { up.m128_f32[0], up.m128_f32[1], up.m128_f32[2] };
}

XMFLOAT3 Transform::GetForward()
{
	if (_wMatDirty) UpdateMatrix();

	XMVECTOR forward = {
		_wMat.r[2].m128_f32[0],
		_wMat.r[2].m128_f32[1],
		_wMat.r[2].m128_f32[2],
		0.0f
	};

	forward = XMVector3Normalize(forward);

	return { forward.m128_f32[0], forward.m128_f32[1], forward.m128_f32[2] };
}


XMMATRIX Transform::GetWorldMatrix()
{
	if (_wMatDirty) UpdateMatrix();

	return _wMat;
}*/
