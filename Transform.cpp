#include "Transform.h"

#include "ErrMsg.h"


void Transform::UpdateMatrix()
{
	_world = 
		XMMatrixScaling(_scale.x, _scale.y, _scale.z) *
		XMMatrixRotationRollPitchYaw(_rot.x, _rot.y, _rot.z) *
		XMMatrixTranslation(_pos.x, _pos.y, _pos.z);

	_dirty = false;
}


void Transform::Move(const XMFLOAT3 &movement)
{
	_pos.x += movement.x;
	_pos.y += movement.y;
	_pos.z += movement.z;

	_dirty = true;
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

	_dirty = true;
}

void Transform::Scale(const XMFLOAT3 &factor)
{
	_scale.x *= factor.x;
	_scale.y *= factor.y;
	_scale.z *= factor.z;

	_dirty = true;
}

void Transform::AxisRotation(const XMFLOAT3 &axis, const float angle)
{
	// TODO: This is incorrect, decomposition gives rotation in quaternion form, not euler angles
	ErrMsg("Transform::AxisRotation() is incorrect, decomposition gives rotation in quaternion form, not euler angles");

	_world =
		XMMatrixScaling(_scale.x, _scale.y, _scale.z) *
		XMMatrixRotationRollPitchYaw(_rot.x, _rot.y, _rot.z) *
		XMMatrixRotationAxis(XMLoadFloat3(&axis), angle) *
		XMMatrixTranslation(_pos.x, _pos.y, _pos.z);

	XMVECTOR scale, rot, pos;
	XMMatrixDecompose(&scale, &rot, &pos, _world);

	_rot.x = rot.m128_f32[0];
	_rot.y = rot.m128_f32[1];
	_rot.z = rot.m128_f32[2];

	_dirty = false;
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
	_dirty = true;
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

	_dirty = true;
}

void Transform::SetScale(const XMFLOAT3 &scale)
{
	_scale = scale;
	_dirty = true;
}


void Transform::SetWorldMatrix(const XMMATRIX &matrix)
{
	// TODO: This is incorrect, decomposition gives rotation in quaternion form, not euler angles
	ErrMsg("Transform::SetWorldMatrix() is incorrect, decomposition gives rotation in quaternion form, not euler angles");

	XMVECTOR scale, rot, pos;
	XMMatrixDecompose(&scale, &rot, &pos, _world);

	_scale.x = scale.m128_f32[0];
	_scale.y = scale.m128_f32[1];
	_scale.z = scale.m128_f32[2];

	_rot.x = rot.m128_f32[0];
	_rot.y = rot.m128_f32[1];
	_rot.z = rot.m128_f32[2];

	_pos.x = pos.m128_f32[0];
	_pos.y = pos.m128_f32[1];
	_pos.z = pos.m128_f32[2];

	_world = matrix;
	_dirty = false;
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
	if (_dirty) UpdateMatrix();

	XMVECTOR right = {
		_world.r[0].m128_f32[0],
		_world.r[0].m128_f32[1],
		_world.r[0].m128_f32[2],
		0.0f
	};

	right = XMVector3Normalize(right);

	return { right.m128_f32[0], right.m128_f32[1], right.m128_f32[2] };
}

XMFLOAT3 Transform::GetUp()
{
	if (_dirty) UpdateMatrix();

	XMVECTOR up = {
		_world.r[1].m128_f32[0],
		_world.r[1].m128_f32[1],
		_world.r[1].m128_f32[2],
		0.0f
	};

	up = XMVector3Normalize(up);

	return { up.m128_f32[0], up.m128_f32[1], up.m128_f32[2] };
}

XMFLOAT3 Transform::GetForward()
{
	if (_dirty) UpdateMatrix();

	XMVECTOR forward = {
		_world.r[2].m128_f32[0],
		_world.r[2].m128_f32[1],
		_world.r[2].m128_f32[2],
		0.0f
	};

	forward = XMVector3Normalize(forward);

	return { forward.m128_f32[0], forward.m128_f32[1], forward.m128_f32[2] };
}


XMMATRIX Transform::GetWorldMatrix()
{
	if (_dirty) UpdateMatrix();

	return _world;
}
