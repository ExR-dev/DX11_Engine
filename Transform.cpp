#include "Transform.h"

#include "ErrMsg.h"


void Transform::Move(const XMFLOAT4A &movement)
{
	_pos.x += movement.x;
	_pos.y += movement.y;
	_pos.z += movement.z;
}

void Transform::Rotate(const XMFLOAT4A &rotation)
{
	RotateByQuaternion(XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z));
}

void Transform::Scale(const XMFLOAT4A &factor)
{
	const XMVECTOR factorVec = XMLoadFloat4A(&factor);

	const float
		rightFactor = XMVectorGetX(XMVector3Dot(*reinterpret_cast<XMVECTOR*>(&_right), factorVec)),
		upFactor = XMVectorGetX(XMVector3Dot(*reinterpret_cast<XMVECTOR*>(&_up), factorVec)),
		forwardFactor = XMVectorGetX(XMVector3Dot(*reinterpret_cast<XMVECTOR*>(&_forward), factorVec));

	_right.x *= rightFactor;
	_right.y *= rightFactor;
	_right.z *= rightFactor;

	_up.x *= upFactor;
	_up.y *= upFactor;
	_up.z *= upFactor;

	_forward.x *= forwardFactor;
	_forward.y *= forwardFactor;
	_forward.z *= forwardFactor;
}

void Transform::MoveLocal(const XMFLOAT4A &movement)
{
	_pos.x += movement.x * _right.x + movement.y * _up.x + movement.z * _forward.x;
	_pos.y += movement.x * _right.y + movement.y * _up.y + movement.z * _forward.y;
	_pos.z += movement.x * _right.z + movement.y * _up.z + movement.z * _forward.z;
}

void Transform::RotateLocal(const XMFLOAT4A &rotation)
{
	RotateByQuaternion(XMQuaternionMultiply(XMQuaternionMultiply(
		XMQuaternionRotationAxis(*reinterpret_cast<XMVECTOR*>(&_right), rotation.x),
			XMQuaternionRotationAxis(*reinterpret_cast<XMVECTOR*>(&_up), rotation.y)),
		XMQuaternionRotationAxis(*reinterpret_cast<XMVECTOR*>(&_forward), rotation.z)));
}

void Transform::ScaleLocal(const XMFLOAT4A &factor)
{
	_right.x *= factor.x;
	_right.y *= factor.x;
	_right.z *= factor.x;

	_up.x *= factor.y;
	_up.y *= factor.y;
	_up.z *= factor.y;

	_forward.x *= factor.z;
	_forward.y *= factor.z;
	_forward.z *= factor.z;
}


void Transform::RotateRoll(const float& rotation)
{
	RotateByQuaternion(XMQuaternionRotationAxis(*reinterpret_cast<XMVECTOR*>(&_forward), rotation));
}

void Transform::RotatePitch(const float& rotation)
{
	RotateByQuaternion(XMQuaternionRotationAxis(*reinterpret_cast<XMVECTOR*>(&_right), rotation));
}

void Transform::RotateYaw(const float& rotation)
{
	RotateByQuaternion(XMQuaternionRotationAxis(*reinterpret_cast<XMVECTOR*>(&_up), rotation));
}


void Transform::RotateByQuaternion(const XMVECTOR &quaternion)
{
	const XMVECTOR
		quatConjugate = XMQuaternionConjugate(quaternion),
		newRight = quaternion * (*reinterpret_cast<XMVECTOR*>(&_right)) * quatConjugate,
		newUp = quaternion * (*reinterpret_cast<XMVECTOR*>(&_up)) * quatConjugate,
		newForward = quaternion * (*reinterpret_cast<XMVECTOR*>(&_forward)) * quatConjugate;

	XMStoreFloat4A(&_right, newRight);
	XMStoreFloat4A(&_up, newUp);
	XMStoreFloat4A(&_forward, newForward);
}


void Transform::SetPosition(const XMFLOAT4A& position)
{
	_pos = position;
}

void Transform::SetAxes(const XMFLOAT4A& right, const XMFLOAT4A& up, const XMFLOAT4A& forward)
{
	_right = right;
	_up = up;
	_forward = forward;
}


XMFLOAT4A Transform::GetPosition() const
{
	return _pos;
}

XMFLOAT4A Transform::GetRight() const
{
	return _right;
}

XMFLOAT4A Transform::GetUp() const
{
	return _up;
}

XMFLOAT4A Transform::GetForward() const
{
	return _forward;
}

XMMATRIX Transform::GetWorldMatrix() const
{
	return XMMatrixSet(
		_right.x, _right.y, _right.z, 0,
		_up.x, _up.y, _up.z, 0,
		_forward.x, _forward.y, _forward.z, 0,
		_pos.x, _pos.y, _pos.z, 1
	);
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
