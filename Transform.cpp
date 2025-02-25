#include "Transform.h"

#include "ErrMsg.h"


using namespace DirectX;


#define TO_VEC(x)		( *reinterpret_cast<XMVECTOR *>(&(x))		)
#define TO_VEC_PTR(x)	(  reinterpret_cast<XMVECTOR *>(&(x))		)
#define TO_CONST_VEC(x) ( *reinterpret_cast<const XMVECTOR *>(&(x))	)


void Transform::NormalizeBases()
{
	XMVECTOR
		*rightPtr = TO_VEC_PTR(_right),
		*upPtr = TO_VEC_PTR(_up),
		*forwardPtr = TO_VEC_PTR(_forward);

	(*rightPtr) = XMVector3Normalize(*rightPtr);
	(*upPtr) = XMVector3Normalize(*upPtr);
	(*forwardPtr) = XMVector3Normalize(*forwardPtr);
}

void Transform::OrthogonalizeBases()
{
	constexpr float epsilon = 0.001f;

	XMVECTOR
		*rightPtr = TO_VEC_PTR(_right),
		*upPtr = TO_VEC_PTR(_up),
		*forwardPtr = TO_VEC_PTR(_forward);

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


void Transform::AddChild(Transform *child)
{
	if (!child)
		return;

	if (!_children.empty())
	{
		auto it = std::find(_children.begin(), _children.end(), child);

		if (it != _children.end())
			return;
	}

	_children.push_back(child);
}

void Transform::RemoveChild(Transform *child)
{
    if (!child)
        return;

    if (_children.empty())
        return;

    auto it = std::find(_children.begin(), _children.end(), child);

    if (it != _children.end())
        _children.erase(it);
}


Transform::Transform(ID3D11Device *device, const XMMATRIX &worldMatrix)
{
	if (!Initialize(device, worldMatrix))
		ErrMsg("Failed to initialize transform from constructor!");
}

Transform::~Transform()
{
	for (auto &child : _children)
	{
		if (child != nullptr)
			child->SetParent(nullptr);
	}
	_children.clear();

	if (_parent)
		_parent->RemoveChild(this);
}


bool Transform::Initialize(ID3D11Device *device, const XMMATRIX& worldMatrix)
{
	TO_VEC(_right) = worldMatrix.r[0];
	TO_VEC(_up) = worldMatrix.r[1];
	TO_VEC(_forward) = worldMatrix.r[2];
	TO_VEC(_pos) = worldMatrix.r[3];

	_scale = {
		XMVectorGetX(XMVector3Length(worldMatrix.r[0])),
		XMVectorGetX(XMVector3Length(worldMatrix.r[1])),
		XMVectorGetX(XMVector3Length(worldMatrix.r[2])),
		0.0f
	};

	NormalizeBases();
	OrthogonalizeBases();

	const XMMATRIX transposeWorldMatrix = XMMatrixTranspose(GetWorldMatrix());
	const XMMATRIX worldMatrixData[2] = {
		transposeWorldMatrix,
		XMMatrixTranspose(XMMatrixInverse(nullptr, transposeWorldMatrix)),
	};

	if (!_worldMatrixBuffer.Initialize(device, sizeof(XMMATRIX) * 2, &worldMatrixData))
	{
		ErrMsg("Failed to initialize world matrix buffer!");
		return false;
	}

	SetDirty();
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


void Transform::SetParent(Transform *parent, bool keepWorldTransform)
{
    if (_parent == parent)
        return;

    XMMATRIX worldMatrix = GetWorldMatrix();

    if (_parent)
        _parent->RemoveChild(this);

    _parent = parent;

    if (parent)
        parent->AddChild(this);

    if (keepWorldTransform)
    {
        XMMATRIX inverseParentWorldMatrix = XMMatrixIdentity();
        if (_parent)
            inverseParentWorldMatrix = XMMatrixInverse(nullptr, _parent->GetWorldMatrix());

        XMMATRIX localMatrix = XMMatrixMultiply(worldMatrix, inverseParentWorldMatrix);
        TO_VEC(_right) = localMatrix.r[0];
        TO_VEC(_up) = localMatrix.r[1];
        TO_VEC(_forward) = localMatrix.r[2];
        TO_VEC(_pos) = localMatrix.r[3];
    }

    SetDirty();
}

Transform *Transform::GetParent() const
{
	return _parent;
}


void Transform::Move(const XMFLOAT4A &movement)
{
	TO_VEC(_pos) += TO_CONST_VEC(movement);

	SetDirty();
}

void Transform::Rotate(const XMFLOAT4A &rotation)
{
	const XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYawFromVector(TO_CONST_VEC(rotation));

	XMVECTOR
		*rightPtr = TO_VEC_PTR(_right),
		*upPtr = TO_VEC_PTR(_up),
		*forwardPtr = TO_VEC_PTR(_forward);

	(*rightPtr) = XMVector3Transform(*rightPtr, rotationMatrix);
	(*upPtr) = XMVector3Transform(*upPtr, rotationMatrix);
	(*forwardPtr) = XMVector3Transform(*forwardPtr, rotationMatrix);

	NormalizeBases();
	OrthogonalizeBases();

	SetDirty();
}

void Transform::ScaleAbsolute(const XMFLOAT4A &scale)
{
	_scale.x += scale.x;
	_scale.y += scale.y;
	_scale.z += scale.z;

	SetDirty();
}


void Transform::MoveLocal(const XMFLOAT4A &movement)
{
	TO_VEC(_pos) += 
		(TO_VEC(_right) * movement.x) + 
		(TO_VEC(_up) * movement.y) +
		(TO_VEC(_forward) * movement.z);

	SetDirty();
}

void Transform::RotateLocal(const XMFLOAT4A &rotation)
{
	XMVECTOR
		*rightPtr = TO_VEC_PTR(_right),
		*upPtr = TO_VEC_PTR(_up),
		*forwardPtr = TO_VEC_PTR(_forward);

	XMMATRIX rotationMatrix = XMMatrixRotationNormal(*forwardPtr, rotation.z);
	rotationMatrix = XMMatrixMultiply(rotationMatrix, XMMatrixRotationNormal(*rightPtr, rotation.x));
	rotationMatrix = XMMatrixMultiply(rotationMatrix, XMMatrixRotationNormal(*upPtr, rotation.y));

	(*rightPtr) = XMVector3Transform(*rightPtr, rotationMatrix);
	(*upPtr) = XMVector3Transform(*upPtr, rotationMatrix);
	(*forwardPtr) = XMVector3Transform(*forwardPtr, rotationMatrix);

	NormalizeBases();
	OrthogonalizeBases();

	SetDirty();
}

void Transform::ScaleRelative(const XMFLOAT4A &scale)
{
	_scale.x *= scale.x;
	_scale.y *= scale.y;
	_scale.z *= scale.z;

	SetDirty();
}


// Unlikely to work, avoid using.
void Transform::RotateByQuaternion(const XMVECTOR &quaternion)
{
	const XMVECTOR
		quatIdentity = XMQuaternionIdentity(),
		quatInverse = XMQuaternionInverse(quaternion);

	XMVECTOR
		*rightPtr = TO_VEC_PTR(_right),
		*upPtr = TO_VEC_PTR(_up),
		*forwardPtr = TO_VEC_PTR(_forward);

	(*rightPtr) = XMQuaternionMultiply(quatIdentity, *rightPtr),
	(*upPtr) = XMQuaternionMultiply(quatIdentity, *upPtr),
	(*forwardPtr) = XMQuaternionMultiply(quatIdentity, *forwardPtr);

	(*rightPtr) = XMQuaternionMultiply(*rightPtr, quatInverse);
	(*upPtr) = XMQuaternionMultiply(*upPtr, quatInverse);
	(*forwardPtr) = XMQuaternionMultiply(*forwardPtr, quatInverse);

	NormalizeBases();
	OrthogonalizeBases();

	SetDirty();
}


void Transform::SetPosition(const XMFLOAT4A &position)
{
	_pos = position;

	SetDirty();
}

void Transform::SetScale(const XMFLOAT4A &scale)
{
	_scale = scale;

	SetDirty();
}

void Transform::SetAxes(const XMFLOAT4A &right, const XMFLOAT4A &up, const XMFLOAT4A &forward)
{
	_right = right;
	_up = up;
	_forward = forward;

	_scale = {
		XMVectorGetX(XMVector3Length(TO_VEC(_right))),
		XMVectorGetX(XMVector3Length(TO_VEC(_up))),
		XMVectorGetX(XMVector3Length(TO_VEC(_forward))),
		0.0f
	};

	NormalizeBases();
	OrthogonalizeBases();

	SetDirty();
}


const XMFLOAT4A &Transform::GetPosition() const	{ return _pos;		}
const XMFLOAT4A &Transform::GetScale() const	{ return _scale;	}
const XMFLOAT4A &Transform::GetRight() const	{ return _right;	}
const XMFLOAT4A &Transform::GetUp() const		{ return _up;		}
const XMFLOAT4A &Transform::GetForward() const	{ return _forward;	}

void Transform::SetDirty()
{
	_isDirty = true;

	for (auto child : _children)
		child->SetDirty();
}

bool Transform::GetDirty() const
{
	return _isDirty;
}


bool Transform::UpdateConstantBuffer(ID3D11DeviceContext *context)
{
	const XMMATRIX transposeWorldMatrix = XMMatrixTranspose(GetWorldMatrix());
	const XMMATRIX worldMatrixData[2] = {
		transposeWorldMatrix,
		XMMatrixTranspose(XMMatrixInverse(nullptr, transposeWorldMatrix)),
	};

	if (!_worldMatrixBuffer.UpdateBuffer(context, &worldMatrixData))
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

XMMATRIX Transform::GetLocalMatrix() const
{
	return XMMatrixSet(
		_right.x * _scale.x,	_right.y * _scale.x,	_right.z * _scale.x,	0,
		_up.x * _scale.y,		_up.y * _scale.y,		_up.z * _scale.y,		0,
		_forward.x * _scale.z,	_forward.y * _scale.z,	_forward.z * _scale.z,	0,
		_pos.x,					_pos.y,					_pos.z,					1
	);
}
XMMATRIX Transform::GetWorldMatrix() const
{
	if (!_parent)
		return GetLocalMatrix();

	return XMMatrixMultiply(GetLocalMatrix(), _parent->GetWorldMatrix());
}
