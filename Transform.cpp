#include "Transform.h"

#include "ErrMsg.h"


Transform::Transform(ID3D11Device *device, const XMMATRIX &worldMatrix)
{
	if (!Initialize(device, worldMatrix))
		ErrMsg("Failed to initialize transform from constructor!");
}


bool Transform::Initialize(ID3D11Device *device, XMMATRIX worldMatrix)
{
	*reinterpret_cast<XMVECTOR *>(&_right) = worldMatrix.r[0];
	*reinterpret_cast<XMVECTOR *>(&_up) = worldMatrix.r[1];
	*reinterpret_cast<XMVECTOR *>(&_forward) = worldMatrix.r[2];
	*reinterpret_cast<XMVECTOR *>(&_pos) = worldMatrix.r[3];

	worldMatrix = XMMatrixTranspose(worldMatrix);
	if (!_worldMatrixBuffer.Initialize(device, sizeof(XMMATRIX), &worldMatrix))
	{
		ErrMsg("Failed to initialize world matrix buffer!");
		return false;
	}

	return true;
}

bool Transform::Initialize(ID3D11Device *device)
{
	const XMMATRIX worldMatrix = GetWorldMatrix();
	if (!Initialize(device, worldMatrix))
	{
		ErrMsg("Failed to initialize world matrix buffer!");
		return false;
	}

	return true;
}


void Transform::Move(const XMFLOAT4A &movement)
{
	/*_pos.x += movement.x;
	_pos.y += movement.y;
	_pos.z += movement.z;*/

	*reinterpret_cast<XMVECTOR *>(&_pos) += *reinterpret_cast<const XMVECTOR *>(&movement);

	_isDirty = true;
}

void Transform::Rotate(const XMFLOAT4A &rotation)
{
	RotateByQuaternion(XMQuaternionRotationRollPitchYawFromVector(*reinterpret_cast<const XMVECTOR *>(&rotation)));
}

void Transform::Scale(const XMFLOAT4A &scaling)
{
	/*const float
		rightScaling = XMVectorGetX(XMVector3Dot(*reinterpret_cast<XMVECTOR *>(&_right), *reinterpret_cast<const XMVECTOR *>(&scaling))),
		upScaling = XMVectorGetX(XMVector3Dot(*reinterpret_cast<XMVECTOR *>(&_up), *reinterpret_cast<const XMVECTOR *>(&scaling))),
		forwardScaling = XMVectorGetX(XMVector3Dot(*reinterpret_cast<XMVECTOR *>(&_forward), *reinterpret_cast<const XMVECTOR *>(&scaling)));

	_right.x *= rightScaling;
	_right.y *= rightScaling;
	_right.z *= rightScaling;

	_up.x *= upScaling;
	_up.y *= upScaling;
	_up.z *= upScaling;

	_forward.x *= forwardScaling;
	_forward.y *= forwardScaling;
	_forward.z *= forwardScaling;*/

	*reinterpret_cast<XMVECTOR *>(&_right) *= XMVectorGetX(XMVector3Dot(
		*reinterpret_cast<XMVECTOR *>(&_right), 
		*reinterpret_cast<const XMVECTOR *>(&scaling)
	));

	*reinterpret_cast<XMVECTOR *>(&_up) *= XMVectorGetX(XMVector3Dot(
		*reinterpret_cast<XMVECTOR *>(&_up), 
		*reinterpret_cast<const XMVECTOR *>(&scaling)
	));

	*reinterpret_cast<XMVECTOR *>(&_forward) *= XMVectorGetX(XMVector3Dot(
		*reinterpret_cast<XMVECTOR *>(&_forward), 
		*reinterpret_cast<const XMVECTOR *>(&scaling)
	));
	
	_isDirty = true;
}

void Transform::MoveLocal(const XMFLOAT4A &movement)
{
	/*_pos.x += movement.x * _right.x + movement.y * _up.x + movement.z * _forward.x;
	_pos.y += movement.x * _right.y + movement.y * _up.y + movement.z * _forward.y;
	_pos.z += movement.x * _right.z + movement.y * _up.z + movement.z * _forward.z;*/

	*reinterpret_cast<XMVECTOR *>(&_pos) += 
		(*reinterpret_cast<XMVECTOR *>(&_right) * movement.x) + 
		(*reinterpret_cast<XMVECTOR *>(&_up) * movement.y) +
		(*reinterpret_cast<XMVECTOR *>(&_forward) * movement.z);

	_isDirty = true;
}

void Transform::RotateLocal(const XMFLOAT4A &rotation)
{
	RotateByQuaternion(XMQuaternionMultiply(XMQuaternionMultiply(
		XMQuaternionRotationAxis(*reinterpret_cast<XMVECTOR *>(&_right), rotation.x),
			XMQuaternionRotationAxis(*reinterpret_cast<XMVECTOR *>(&_up), rotation.y)),
		XMQuaternionRotationAxis(*reinterpret_cast<XMVECTOR *>(&_forward), rotation.z)));
}

void Transform::ScaleLocal(const XMFLOAT4A &scaling)
{
	/*_right.x *= scaling.x;
	_right.y *= scaling.x;
	_right.z *= scaling.x;

	_up.x *= scaling.y;
	_up.y *= scaling.y;
	_up.z *= scaling.y;

	_forward.x *= scaling.z;
	_forward.y *= scaling.z;
	_forward.z *= scaling.z;*/

	*reinterpret_cast<XMVECTOR *>(&_right) *= scaling.x;
	*reinterpret_cast<XMVECTOR *>(&_up) *= scaling.y;
	*reinterpret_cast<XMVECTOR *>(&_forward) *= scaling.z;

	_isDirty = true;
}


void Transform::RotateRoll(const float rotation)
{
	RotateByQuaternion(XMQuaternionRotationAxis(*reinterpret_cast<XMVECTOR *>(&_forward), rotation));
}

void Transform::RotatePitch(const float rotation)
{
	RotateByQuaternion(XMQuaternionRotationAxis(*reinterpret_cast<XMVECTOR *>(&_right), rotation));
}

void Transform::RotateYaw(const float rotation)
{
	RotateByQuaternion(XMQuaternionRotationAxis(*reinterpret_cast<XMVECTOR *>(&_up), rotation));
}


void Transform::RotateByQuaternion(const XMVECTOR &quaternion)
{
	const XMVECTOR
		quatConjugate = XMQuaternionConjugate(quaternion),
		newRight = quaternion * *reinterpret_cast<XMVECTOR *>(&_right) * quatConjugate,
		newUp = quaternion * *reinterpret_cast<XMVECTOR *>(&_up) * quatConjugate,
		newForward = quaternion * *reinterpret_cast<XMVECTOR *>(&_forward) * quatConjugate;

	XMStoreFloat4A(&_right, newRight);
	XMStoreFloat4A(&_up, newUp);
	XMStoreFloat4A(&_forward, newForward);

	_isDirty = true;
}


void Transform::SetPosition(const XMFLOAT4A &position)
{
	_pos = position;

	_isDirty = true;
}

void Transform::SetAxes(const XMFLOAT4A &right, const XMFLOAT4A &up, const XMFLOAT4A &forward)
{
	_right = right;
	_up = up;
	_forward = forward;

	_isDirty = true;
}

void Transform::SetScale(const XMFLOAT4A &scale)
{
	*reinterpret_cast<XMVECTOR *>(&_right) = XMVector3Normalize(*reinterpret_cast<XMVECTOR *>(&_right)) * scale.x;
	*reinterpret_cast<XMVECTOR *>(&_up) = XMVector3Normalize(*reinterpret_cast<XMVECTOR *>(&_up)) * scale.y;
	*reinterpret_cast<XMVECTOR *>(&_forward) = XMVector3Normalize(*reinterpret_cast<XMVECTOR *>(&_forward)) * scale.z;

	_isDirty = true;
}


XMFLOAT4A Transform::GetPosition() const { return _pos;		}
XMFLOAT4A Transform::GetRight() const	 { return _right;	}
XMFLOAT4A Transform::GetUp() const		 { return _up;		}
XMFLOAT4A Transform::GetForward() const	 { return _forward; }


bool Transform::UpdateConstantBuffer(ID3D11DeviceContext *context)
{
	XMFLOAT4X4 worldMatrix;
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(GetWorldMatrix()));

	if (!_worldMatrixBuffer.UpdateBuffer(context, &worldMatrix))
	{
		ErrMsg("Failed to update world matrix buffer!");
		return false;
	}

	_isDirty = false;
	return true;
}

ID3D11Buffer *Transform::GetConstantBuffer() const
{
	return _worldMatrixBuffer.GetBuffer();
}

XMMATRIX Transform::GetWorldMatrix() const
{
	return XMMatrixSet(
		_right.x,	_right.y,	_right.z,	0,
		_up.x,		_up.y,		_up.z,		0,
		_forward.x, _forward.y, _forward.z, 0,
		_pos.x,		_pos.y,		_pos.z,		1
	);
}
