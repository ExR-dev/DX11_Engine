#include "Transform.h"

#include "ErrMsg.h"


void Transform::NormalizeBases()
{
	XMVECTOR
		*rightPtr = reinterpret_cast<XMVECTOR *>(&_right),
		*upPtr = reinterpret_cast<XMVECTOR *>(&_up),
		*forwardPtr = reinterpret_cast<XMVECTOR *>(&_forward);

	(*rightPtr) = XMVector3Normalize(*rightPtr);
	(*upPtr) = XMVector3Normalize(*upPtr);
	(*forwardPtr) = XMVector3Normalize(*forwardPtr);
}

void Transform::OrthogonalizeBases()
{
	constexpr float epsilon = 0.001f;

	XMVECTOR
		*rightPtr = reinterpret_cast<XMVECTOR *>(&_right),
		*upPtr = reinterpret_cast<XMVECTOR *>(&_up),
		*forwardPtr = reinterpret_cast<XMVECTOR *>(&_forward);

	if (abs(XMVectorGetX(XMVector3Dot(*forwardPtr, *upPtr))) > epsilon)
	{
		ErrMsg("Forward and up vectors not orthogonal! Attempting to fix.");

		// Apply Gram-Schmidt process
		(*upPtr) = XMVector3Normalize(XMVectorSubtract(*upPtr, XMVector3Dot(*upPtr, *forwardPtr) * (*forwardPtr)));
	}

	if (abs(XMVectorGetX(XMVector3Dot(*forwardPtr, *rightPtr))) > epsilon ||
		abs(XMVectorGetX(XMVector3Dot(*upPtr, *rightPtr))) > epsilon)
	{
		ErrMsg("Right vector is not orthogonal! Attempting to fix.");

		// Recalculate right vector with cross product
		(*rightPtr) = XMVector3Normalize(XMVector3Cross(*forwardPtr, *upPtr));
	}
}


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

	_scale = {
		XMVectorGetX(XMVector3Length(worldMatrix.r[0])),
		XMVectorGetX(XMVector3Length(worldMatrix.r[1])),
		XMVectorGetX(XMVector3Length(worldMatrix.r[2])),
		0.0f
	};

	NormalizeBases();
	OrthogonalizeBases();

	worldMatrix = XMMatrixTranspose(GetWorldMatrix());
	if (!_worldMatrixBuffer.Initialize(device, sizeof(XMMATRIX), &worldMatrix))
	{
		ErrMsg("Failed to initialize world matrix buffer!");
		return false;
	}

	_isDirty = true;
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
	*reinterpret_cast<XMVECTOR *>(&_pos) += *reinterpret_cast<const XMVECTOR *>(&movement);

	_isDirty = true;
}

void Transform::Rotate(const XMFLOAT4A &rotation)
{
	const XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYawFromVector(*reinterpret_cast<const XMVECTOR *>(&rotation));

	XMVECTOR
		*rightPtr = reinterpret_cast<XMVECTOR *>(&_right),
		*upPtr = reinterpret_cast<XMVECTOR *>(&_up),
		*forwardPtr = reinterpret_cast<XMVECTOR *>(&_forward);

	(*rightPtr) = XMVector3Transform(*rightPtr, rotationMatrix);
	(*upPtr) = XMVector3Transform(*upPtr, rotationMatrix);
	(*forwardPtr) = XMVector3Transform(*forwardPtr, rotationMatrix);

	NormalizeBases();
	OrthogonalizeBases();

	_isDirty = true;
}

void Transform::ScaleAbsolute(const XMFLOAT4A &scale)
{
	_scale.x += scale.x;
	_scale.y += scale.y;
	_scale.z += scale.z;

	_isDirty = true;
}


void Transform::MoveLocal(const XMFLOAT4A &movement)
{
	*reinterpret_cast<XMVECTOR *>(&_pos) += 
		(*reinterpret_cast<XMVECTOR *>(&_right) * movement.x) + 
		(*reinterpret_cast<XMVECTOR *>(&_up) * movement.y) +
		(*reinterpret_cast<XMVECTOR *>(&_forward) * movement.z);

	_isDirty = true;
}

void Transform::RotateLocal(const XMFLOAT4A &rotation)
{
	XMVECTOR
		*rightPtr = reinterpret_cast<XMVECTOR *>(&_right),
		*upPtr = reinterpret_cast<XMVECTOR *>(&_up),
		*forwardPtr = reinterpret_cast<XMVECTOR *>(&_forward);

	XMMATRIX rotationMatrix = XMMatrixRotationNormal(*forwardPtr, rotation.z);
	rotationMatrix = XMMatrixMultiply(rotationMatrix, XMMatrixRotationNormal(*rightPtr, rotation.x));
	rotationMatrix = XMMatrixMultiply(rotationMatrix, XMMatrixRotationNormal(*upPtr, rotation.y));

	(*rightPtr) = XMVector3Transform(*rightPtr, rotationMatrix);
	(*upPtr) = XMVector3Transform(*upPtr, rotationMatrix);
	(*forwardPtr) = XMVector3Transform(*forwardPtr, rotationMatrix);

	NormalizeBases();
	OrthogonalizeBases();

	_isDirty = true;
}

void Transform::ScaleRelative(const XMFLOAT4A &scale)
{
	_scale.x *= scale.x;
	_scale.y *= scale.y;
	_scale.z *= scale.z;

	_isDirty = true;
}


// Unlikely to work, avoid using.
void Transform::RotateByQuaternion(const XMVECTOR &quaternion)
{
	const XMVECTOR
		quatIdentity = XMQuaternionIdentity(),
		quatInverse = XMQuaternionInverse(quaternion);

	XMVECTOR
		*rightPtr = reinterpret_cast<XMVECTOR *>(&_right),
		*upPtr = reinterpret_cast<XMVECTOR *>(&_up),
		*forwardPtr = reinterpret_cast<XMVECTOR *>(&_forward);

	(*rightPtr) = XMQuaternionMultiply(quatIdentity, *rightPtr),
	(*upPtr) = XMQuaternionMultiply(quatIdentity, *upPtr),
	(*forwardPtr) = XMQuaternionMultiply(quatIdentity, *forwardPtr);

	(*rightPtr) = XMQuaternionMultiply(*rightPtr, quatInverse);
	(*upPtr) = XMQuaternionMultiply(*upPtr, quatInverse);
	(*forwardPtr) = XMQuaternionMultiply(*forwardPtr, quatInverse);

	NormalizeBases();
	OrthogonalizeBases();

	_isDirty = true;
}


void Transform::SetPosition(const XMFLOAT4A &position)
{
	_pos = position;

	_isDirty = true;
}

void Transform::SetScale(const XMFLOAT4A &scale)
{
	_scale = scale;

	_isDirty = true;
}

void Transform::SetAxes(const XMFLOAT4A &right, const XMFLOAT4A &up, const XMFLOAT4A &forward)
{
	_right = right;
	_up = up;
	_forward = forward;

	_scale = {
		XMVectorGetX(XMVector3Length(*reinterpret_cast<XMVECTOR *>(&_right))),
		XMVectorGetX(XMVector3Length(*reinterpret_cast<XMVECTOR *>(&_up))),
		XMVectorGetX(XMVector3Length(*reinterpret_cast<XMVECTOR *>(&_forward))),
		0.0f
	};

	NormalizeBases();
	OrthogonalizeBases();

	_isDirty = true;
}


const XMFLOAT4A &Transform::GetPosition() const	{ return _pos;		}
const XMFLOAT4A &Transform::GetScale() const	{ return _scale;	}
const XMFLOAT4A &Transform::GetRight() const	{ return _right;	}
const XMFLOAT4A &Transform::GetUp() const		{ return _up;		}
const XMFLOAT4A &Transform::GetForward() const	{ return _forward;	}


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
		_right.x * _scale.x,	_right.y * _scale.x,	_right.z * _scale.x,	0,
		_up.x * _scale.y,		_up.y * _scale.y,		_up.z * _scale.y,		0,
		_forward.x * _scale.z,	_forward.y * _scale.z,	_forward.z * _scale.z,  0,
		_pos.x,					_pos.y,					_pos.z,		1
	);
}
