#include "Transform.h"
#include "ErrMsg.h"

using namespace DirectX;
using namespace SimpleMath;

#define TO_VEC(x)		(*reinterpret_cast<XMVECTOR *>(&(x)))
#define TO_VEC_PTR(x)	( reinterpret_cast<XMVECTOR *>(&(x)))
#define TO_CONST_VEC(x)	(*reinterpret_cast<const XMVECTOR *>(&(x)))


Transform::Transform(ID3D11Device *device, const Matrix &worldMatrix)
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

bool Transform::Initialize(ID3D11Device *device, const Matrix &worldMatrix)
{
	worldMatrix.Decompose(_localScale, _localRotation, _localPosition);

	const Matrix transposeWorldMatrix = GetWorldMatrix().Transpose();
	Matrix inverseTransposeWorldMatrix;
	transposeWorldMatrix.Invert(inverseTransposeWorldMatrix);

	const Matrix worldMatrixData[2] = {
		transposeWorldMatrix,
		transposeWorldMatrix.Transpose(),
	};

	if (!_worldMatrixBuffer.Initialize(device, sizeof(Matrix) * 2, &worldMatrixData))
	{
		ErrMsg("Failed to initialize world matrix buffer!");
		return false;
	}

	SetDirty();
	return true;
}
bool Transform::Initialize(ID3D11Device *device)
{
	if (!Initialize(device, GetWorldMatrix()))
	{
		ErrMsg("Failed to initialize world matrix buffer!");
		return false;
	}

	return true;
}

inline void Transform::AddChild(Transform *child)
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
inline void Transform::RemoveChild(Transform *child)
{
    if (!child)
        return;

    if (_children.empty())
        return;

    auto it = std::find(_children.begin(), _children.end(), child);

    if (it != _children.end())
        _children.erase(it);
}

void Transform::SetWorldPositionDirty()
{
	_isWorldPositionDirty = true;
	_isWorldMatrixDirty = true;
	_isDirty = true;

	for (auto child : _children)
	{
		child->SetWorldPositionDirty();
	}
}
void Transform::SetWorldRotationDirty()
{
	_isWorldRotationDirty = true;
	_isWorldMatrixDirty = true;
	_isDirty = true;

	for (auto child : _children)
	{
		child->_isWorldPositionDirty = true;
		child->_isWorldScaleDirty = true;
		child->SetWorldRotationDirty();
	}
}
void Transform::SetWorldScaleDirty()
{
	_isWorldScaleDirty = true;
	_isWorldMatrixDirty = true;
	_isDirty = true;

	for (auto child : _children)
	{
		child->_isWorldPositionDirty = true;
		child->SetWorldScaleDirty();
	}
}
void Transform::SetAllDirty()
{
	_isWorldPositionDirty = true;
	_isWorldRotationDirty = true;
	_isWorldScaleDirty = true;
	_isLocalMatrixDirty = true;
	_isWorldMatrixDirty = true;
	_isDirty = true;

	for (auto child : _children)
	{
		child->SetAllDirty();
	}
}
void Transform::SetDirty()
{
	_isDirty = true;

	for (auto child : _children)
		if (!child->_isDirty)
			child->SetDirty();
}
bool Transform::IsDirty() const
{
	return _isDirty;
}

Vector3 *Transform::WorldPosition()
{
	if (!_isWorldPositionDirty)
		return &_worldPosition;
	
	// Recalculate.
	// _worldPosition = WorldMatrix()->Translation();

	Vector3 worldPos = _localPosition;

	Transform *iter = _parent;
	while (iter != nullptr) 
	{
		worldPos = worldPos * iter->_localScale;
		worldPos = worldPos * iter->_localRotation;
		worldPos += iter->_localPosition;
		iter = iter->_parent;
	}

	_worldPosition = worldPos;

	_isWorldPositionDirty = false;
	return &_worldPosition;
}
Quaternion *Transform::WorldRotation()
{
	if (!_isWorldRotationDirty)
		return &_worldRotation;

	// Recalculate.
	FXMMATRIX M = *WorldMatrix();
	XMMATRIX matTemp{};

	matTemp.r[0] = M.r[0];
	matTemp.r[1] = M.r[1];
	matTemp.r[2] = M.r[2];
	matTemp.r[3] = g_XMIdentityR3.v;

	_worldRotation = XMQuaternionRotationMatrix(matTemp);

	_isWorldRotationDirty = false;
	return &_worldRotation;
}
Vector3 *Transform::WorldScale()
{
	if (!_isWorldScaleDirty)
		return &_worldScale;

	// Recalculate.
	Matrix *worldMatrix = WorldMatrix();

	_worldScale = Vector3(
		worldMatrix->Right().Length(),
		worldMatrix->Up().Length(),
		worldMatrix->Forward().Length()
	);

	_isWorldScaleDirty = false;
	return &_worldScale;
}
Matrix *Transform::WorldMatrix()
{
	if (!_isWorldMatrixDirty)
		return &_worldMatrix;
	
	// Recalculate.
	if (!_parent)
		return LocalMatrix();

	_worldMatrix = (*LocalMatrix()) * (*_parent->WorldMatrix());

	_isWorldMatrixDirty = false;
	return &_worldMatrix;
}
Matrix *Transform::LocalMatrix()
{
	if (!_isLocalMatrixDirty)
		return &_localMatrix;

	// Recalculate.
	const Vector3 t = _localPosition;

	Vector3 x = Vector3::Transform(Vector3::Right, _localRotation);
	Vector3 y = Vector3::Transform(Vector3::Up, _localRotation);
	Vector3 z = Vector3::Transform(Vector3::Forward, _localRotation);

	x = x * _localScale.x;
	y = y * _localScale.y;
	z = z * _localScale.z;

	_localMatrix = Matrix(
		x.x, x.y, x.z, 0,
		y.x, y.y, y.z, 0,
		z.x, z.y, z.z, 0,
		t.x, t.y, t.z, 1
	);

	_isLocalMatrixDirty = false;
	return &_localMatrix;
}

void Transform::SetParent(Transform *newParent, bool keepWorldTransform)
{
    if (_parent == newParent)
        return;

    /*XMMATRIX worldMatrix;
	if (keepWorldTransform)
		worldMatrix = GetWorldMatrix();*/

    if (_parent)
		_parent->RemoveChild(this);

    _parent = newParent;

    if (newParent)
		newParent->AddChild(this);

    /*if (keepWorldTransform)
    {
        XMMATRIX inverseParentWorldMatrix = _parent ? 
			XMMatrixInverse(nullptr, _parent->GetWorldMatrix()) : 
			XMMatrixIdentity();

        XMMATRIX localMatrix = XMMatrixMultiply(worldMatrix, inverseParentWorldMatrix);
        TO_VEC(_right) = localMatrix.r[0];
        TO_VEC(_up) = localMatrix.r[1];
        TO_VEC(_forward) = localMatrix.r[2];
        TO_VEC(_pos) = localMatrix.r[3];
    }*/

	SetAllDirty();
}
Transform *Transform::GetParent() const
{
	return _parent;
}

const Vector3 Transform::Right(ReferenceSpace space)
{
	if (!_parent)
		space = Local;

	switch (space)
	{
	case Local:
		return Vector3::Transform(Vector3::Right, _localRotation);

	case World:
		return WorldMatrix()->Right();

	default:
		ErrMsg("Invalid reference space!");
		return Vector3::Right;
	}
}
const Vector3 Transform::Up(ReferenceSpace space)
{
	if (!_parent)
		space = Local;

	switch (space)
	{
	case Local:
		return Vector3::Transform(Vector3::Up, _localRotation);

	case World:
		return WorldMatrix()->Up();

	default:
		ErrMsg("Invalid reference space!");
		return Vector3::Up;
	}
}
const Vector3 Transform::Forward(ReferenceSpace space)
{
	if (!_parent)
		space = Local;

	switch (space)
	{
	case Local:
		return Vector3::Transform(Vector3::Forward, _localRotation);

	case World:
		return WorldMatrix()->Forward();

	default:
		ErrMsg("Invalid reference space!");
		return Vector3::Forward;
	}
}

const Vector3 &Transform::GetPosition(ReferenceSpace space)
{
	if (!_parent)
		space = Local;

	switch (space)
	{
	case Local:
		return _localPosition;

	case World:
		return *WorldPosition();

	default:
		ErrMsg("Invalid reference space!");
		return Vector3::Zero;
	}
}
const Quaternion &Transform::GetRotation(ReferenceSpace space)
{
	if (!_parent)
		space = Local;

	switch (space)
	{
	case Local:
		return _localRotation;

	case World:
		return *WorldRotation();

	default:
		ErrMsg("Invalid reference space!");
		return Quaternion::Identity;
	}
}
const Vector3 &Transform::GetScale(ReferenceSpace space)
{
	if (!_parent)
		space = Local;

	switch (space)
	{
	case Local:
		return _localScale;

	case World:
		return *WorldScale();

	default:
		ErrMsg("Invalid reference space!");
		return Vector3::One;
	}
}

const Vector3 Transform::InverseTransformPoint(Vector3 &point) const
{
	if (_parent)
		point = _parent->InverseTransformPoint(point);

	// First, apply the inverse translation of the transform
	point = point - _localPosition;

	// Next, apply the inverse rotation of the transform
	Quaternion invRot;
	_localRotation.Inverse(invRot);

	point = point * invRot;
	return point / _localScale;
}
const Matrix Transform::GetGlobalRotationAndScale()
{
	Matrix worldRS = Matrix::CreateFromQuaternion(_localRotation) * Matrix::CreateScale(_localScale);

	if (_parent) 
	{
		Matrix parentRS = _parent->GetGlobalRotationAndScale();
		worldRS = parentRS * worldRS;
	}

	return worldRS;
}

void Transform::SetPosition(const Vector3 &position, ReferenceSpace space)
{
	if (!_parent)
		space = Local;

	switch (space)
	{
	case Local:
		_localPosition = position;
		break;

	case World:
	{
		Vector3 point = position;
		_localPosition = InverseTransformPoint(point);
		break;
	}

	default:
		ErrMsg("Invalid reference space!");
		break;
	}

	SetWorldPositionDirty();
	_isLocalMatrixDirty = true;
}
void Transform::SetRotation(const Quaternion &rotation, ReferenceSpace space)
{
	if (!_parent)
		space = Local;

	switch (space)
	{
	case Local:
		_localRotation = rotation;
		break;

	case World:
	{
		Quaternion invParentGlobal;
		_parent->WorldRotation()->Inverse(invParentGlobal);

		_localRotation = Quaternion::Concatenate(invParentGlobal, rotation);
		break;
	}

	default:
		ErrMsg("Invalid reference space!");
		break;
	}

	SetWorldRotationDirty();
	_isLocalMatrixDirty = true;
}
void Transform::SetScale(const Vector3 &scale, ReferenceSpace space)
{
	if (!_parent)
		space = Local;

	switch (space)
	{
	case Local:
		_localScale = scale;
		break;

	case World:
	{
		const Quaternion globalRotation = *WorldRotation();

		const Vector3 x = Vector3(scale.x, 0, 0) * globalRotation;
		const Vector3 y = Vector3(0, scale.y, 0) * globalRotation;
		const Vector3 z = Vector3(0, 0, scale.z) * globalRotation;

		const Matrix rsMat = Matrix(
			x.x, x.y, x.z, 0,
			y.x, y.y, y.z, 0,
			z.x, z.y, z.z, 0,
			0, 0, 0, 1
		);

		_localScale = Vector3(1, 1, 1);

		const Matrix inverseRS = GetGlobalRotationAndScale().Invert();
		const Matrix localRS = inverseRS * rsMat;

		// Main diagonal is the new scale
		_localScale = Vector3(localRS._11, localRS._22, localRS._33);
		break;
	}

	default:
		ErrMsg("Invalid reference space!");
		break;
	}

	SetWorldScaleDirty();
	_isLocalMatrixDirty = true;
}

void Transform::Move(const DirectX::SimpleMath::Vector3 &direction, ReferenceSpace space)
{
	switch (space)
	{
	case Local:
		_localPosition += direction;
		break;

	case World:
		SetPosition((*WorldPosition()) + direction, World);
		break;

	default:
		ErrMsg("Invalid reference space!");
		break;
	}

	SetWorldPositionDirty();
	_isLocalMatrixDirty = true;
}
void Transform::Rotate(const DirectX::SimpleMath::Vector3 &euler, ReferenceSpace space)
{
	switch (space)
	{
	case Local:
		_localRotation = Quaternion::Concatenate(_localRotation, Quaternion::CreateFromYawPitchRoll(euler));
		break;

	case World:
		SetRotation(Quaternion::Concatenate(*WorldRotation(), Quaternion::CreateFromYawPitchRoll(euler)), World);
		break;

	default:
		ErrMsg("Invalid reference space!");
		break;
	}

	SetWorldRotationDirty();
	_isLocalMatrixDirty = true;
}

const Vector3 Transform::GetEuler(ReferenceSpace space)
{
	return GetRotation(space).ToEuler();
}
void Transform::SetEuler(const DirectX::SimpleMath::Vector3 &rollPitchYaw, ReferenceSpace space)
{
	SetRotation(Quaternion::CreateFromYawPitchRoll(rollPitchYaw), space);
}

bool Transform::UpdateConstantBuffer(ID3D11DeviceContext *context)
{
	const Matrix transposeWorldMatrix = GetWorldMatrix().Transpose();
	Matrix inverseTransposeWorldMatrix;
	transposeWorldMatrix.Invert(inverseTransposeWorldMatrix);

	const Matrix worldMatrixData[2] = {
		transposeWorldMatrix,
		transposeWorldMatrix.Transpose(),
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

Matrix Transform::GetLocalMatrix()
{
	return *LocalMatrix();
}
Matrix Transform::GetWorldMatrix()
{
	return *WorldMatrix();
}
