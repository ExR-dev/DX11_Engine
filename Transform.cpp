#include "Transform.h"
#include "ErrMsg.h"

using namespace DirectX;

#define TO_VEC(x)		(*reinterpret_cast<XMVECTOR *>(&(x)))
#define TO_VEC_PTR(x)	( reinterpret_cast<XMVECTOR *>(&(x)))
#define TO_CONST_VEC(x)	(*reinterpret_cast<const XMVECTOR *>(&(x)))


Transform::Transform(ID3D11Device *device, const XMFLOAT4X4A &worldMatrix)
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

bool Transform::Initialize(ID3D11Device *device, const XMFLOAT4X4A &worldMatrix)
{
	XMVECTOR s, r, p;

	XMMatrixDecompose(&s, &r, &p, Load(worldMatrix));

	Store(_localScale, s);
	Store(_localRotation, r);
	Store(_localPosition, p);

	const XMMATRIX transposeWorldMatrix = XMMatrixTranspose(Load(*WorldMatrix()));
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
	if (!Initialize(device, GetWorldMatrix()))
	{
		ErrMsg("Failed to initialize world matrix buffer!");
		return false;
	}

	return true;
}

const XMFLOAT4A Transform::To4(const XMFLOAT3A &vec) const
{
	return *reinterpret_cast<const XMFLOAT4A *>(&vec);
}
const XMFLOAT3A Transform::To3(const XMFLOAT4A &vec) const
{
	return *reinterpret_cast<const XMFLOAT3A *>(&vec);
}

inline XMVECTOR Transform::Load(const XMFLOAT3A &float3A) const
{
	return XMLoadFloat3A(&float3A);
}
inline XMVECTOR Transform::Load(const XMFLOAT4A &float4A) const
{
	return XMLoadFloat4A(&float4A);
}
inline XMMATRIX Transform::Load(const XMFLOAT4X4A &float4x4A) const
{
	return XMLoadFloat4x4A(&float4x4A);
}
inline XMVECTOR Transform::Load(const XMFLOAT3A *float3A) const
{
	return XMLoadFloat3A(float3A);
}
inline XMVECTOR Transform::Load(const XMFLOAT4A *float4A) const
{
	return XMLoadFloat4A(float4A);
}
inline XMMATRIX Transform::Load(const XMFLOAT4X4A *float4x4A) const
{
	return XMLoadFloat4x4A(float4x4A);
}
inline void Transform::Store(XMFLOAT3A &dest, const XMVECTOR &vec) const
{
	XMStoreFloat3A(&dest, vec);
}
inline void Transform::Store(XMFLOAT4A &dest, const XMVECTOR &vec) const
{
	XMStoreFloat4A(&dest, vec);
}
inline void Transform::Store(XMFLOAT4X4A &dest, const XMMATRIX &mat) const
{
	XMStoreFloat4x4A(&dest, mat);
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

XMFLOAT3A *Transform::WorldPosition()
{
	if (!_isWorldPositionDirty)
		return &_worldPosition;
	
	// Recalculate.
	auto v = WorldMatrix()->m[3];
	_worldPosition = { v[0], v[1], v[2] };

	/*
	XMVECTOR worldPos = Load(_localPosition);

	Transform *iter = _parent;
	while (iter != nullptr) 
	{
		worldPos = XMVectorMultiply(worldPos, Load(iter->_localScale));
		worldPos = XMVector3Rotate(worldPos, Load(iter->_localRotation));
		worldPos = XMVectorAdd(worldPos, Load(iter->_localPosition));
		iter = iter->_parent;
	}

	Store(_worldPosition, worldPos);*/

	_isWorldPositionDirty = false;
	return &_worldPosition;
}
XMFLOAT4A *Transform::WorldRotation()
{
 	if (!_isWorldRotationDirty)
		return &_worldRotation;

	// Recalculate.
	FXMMATRIX M = Load(WorldMatrix());
	XMMATRIX matTemp{};

	matTemp.r[0] = M.r[0];
	matTemp.r[1] = M.r[1];
	matTemp.r[2] = M.r[2];
	matTemp.r[3] = g_XMIdentityR3.v;

	Store(_worldRotation, XMQuaternionRotationMatrix(matTemp));

	_isWorldRotationDirty = false;
	return &_worldRotation;
}
XMFLOAT3A *Transform::WorldScale()
{
	if (!_isWorldScaleDirty)
		return &_worldScale;

	// Recalculate.
	XMMATRIX worldMatrix = Load(WorldMatrix());

	_worldScale = XMFLOAT3A(
		XMVectorGetX(XMVector3Length(worldMatrix.r[0])),
		XMVectorGetX(XMVector3Length(worldMatrix.r[1])),
		XMVectorGetX(XMVector3Length(worldMatrix.r[2]))
	);

	_isWorldScaleDirty = false;
	return &_worldScale;
}
XMFLOAT4X4A *Transform::WorldMatrix()
{
	if (!_isWorldMatrixDirty)
		return &_worldMatrix;
	
	// Recalculate.
	if (!_parent)
		_worldMatrix = *LocalMatrix();
	else
		Store(_worldMatrix, XMMatrixMultiply(Load(*LocalMatrix()), Load(*_parent->WorldMatrix())));

	_isWorldMatrixDirty = false;
	return &_worldMatrix;
}
XMFLOAT4X4A *Transform::LocalMatrix()
{
	if (!_isLocalMatrixDirty)
		return &_localMatrix;

	// Recalculate.
	XMFLOAT3A x;
	XMFLOAT3A y;
	XMFLOAT3A z;

	XMVECTOR localRot = Load(_localRotation);

	Store(x, XMVector3Rotate({1, 0, 0, 0}, localRot));
	Store(y, XMVector3Rotate({0, 1, 0, 0}, localRot));
	Store(z, XMVector3Rotate({0, 0, 1, 0}, localRot));

	x = { x.x * _localScale.x, x.y * _localScale.x, x.z * _localScale.x };
	y = { y.x * _localScale.y, y.y * _localScale.y, y.z * _localScale.y };
	z = { z.x * _localScale.z, z.y * _localScale.z, z.z * _localScale.z };

	const XMFLOAT3A t = _localPosition;
	_localMatrix = XMFLOAT4X4A(
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

    if (_parent)
		_parent->RemoveChild(this);

    _parent = newParent;

    if (newParent)
		newParent->AddChild(this);

	SetAllDirty();
}
Transform *Transform::GetParent() const
{
	return _parent;
}

const XMFLOAT3A &Transform::GetRight(ReferenceSpace space)
{
	if (!_parent)
		space = Local;

	switch (space)
	{
	case Local:
	{
		auto *v = LocalMatrix()->m[0];
		return { v[0], v[1], v[2] };
	}

	case World:
	{
		auto *v = WorldMatrix()->m[0];
		return { v[0], v[1], v[2] };
	}

	default:
		ErrMsg("Invalid reference space!");
	}
}
const XMFLOAT3A &Transform::GetUp(ReferenceSpace space)
{
	if (!_parent)
		space = Local;

	switch (space)
	{
	case Local:
	{
		auto *v = LocalMatrix()->m[1];
		return { v[0], v[1], v[2] };
	}

	case World:
	{
		auto *v = WorldMatrix()->m[1];
		return { v[0], v[1], v[2] };
	}

	default:
		ErrMsg("Invalid reference space!");
	}
}
const XMFLOAT3A &Transform::GetForward(ReferenceSpace space)
{
	if (!_parent)
		space = Local;

	switch (space)
	{
	case Local:
	{
		auto *v = LocalMatrix()->m[2];
		return { v[0], v[1], v[2] };
	}

	case World:
	{
		auto *v = WorldMatrix()->m[2];
		return { v[0], v[1], v[2] };
	}

	default:
		ErrMsg("Invalid reference space!");
	}
}
void Transform::GetAxes(XMFLOAT3A *right, XMFLOAT3A *up, XMFLOAT3A *forward, ReferenceSpace space)
{
	if (!_parent)
		space = Local;

	switch (space)
	{
	case Local:
	{
		auto m = LocalMatrix()->m;
		
		if (right)
			memcpy(right, m[0], sizeof(XMFLOAT3));
		if (up)
			memcpy(up, m[1], sizeof(XMFLOAT3));
		if (forward)
			memcpy(forward, m[2], sizeof(XMFLOAT3));
		break;
	}

	case World:
	{
		auto m = WorldMatrix()->m;

		if (right)
			memcpy(right, m[0], sizeof(XMFLOAT3));
		if (up)
			memcpy(up, m[1], sizeof(XMFLOAT3));
		if (forward)
			memcpy(forward, m[2], sizeof(XMFLOAT3));
		break;
	}

	default:
		ErrMsg("Invalid reference space!");
		break;
	}
}

const XMFLOAT3A &Transform::GetPosition(ReferenceSpace space)
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
	}
}
const XMFLOAT4A &Transform::GetRotation(ReferenceSpace space)
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
	}
}
const XMFLOAT3A &Transform::GetScale(ReferenceSpace space)
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
	}
}

const XMFLOAT3A Transform::InverseTransformPoint(XMFLOAT3A &point) const
{
	if (_parent)
		point = _parent->InverseTransformPoint(point);

	XMVECTOR pointVec = Load(point);

	pointVec = XMVectorSubtract(pointVec, Load(_localPosition));

	XMVECTOR invRot = XMQuaternionInverse(Load(_localRotation));

	pointVec = XMVector3Rotate(pointVec, invRot);

	Store(point, XMVectorDivide(pointVec, Load(_localScale)));
	return point;
}
const XMFLOAT4X4A Transform::GetWorldRotationAndScale()
{
	XMMATRIX worldRS = XMMatrixMultiply(XMMatrixRotationQuaternion(Load(_localRotation)), XMMatrixScaling(_localScale.x, _localScale.y, _localScale.z));

	if (_parent) 
	{
		XMMATRIX parentRS = Load(_parent->GetWorldRotationAndScale());
		worldRS = parentRS * worldRS;
	}

	XMFLOAT4X4A storedWorldRS;
	Store(storedWorldRS, worldRS);

	return storedWorldRS;
}

void Transform::SetPosition(const XMFLOAT3A &position, ReferenceSpace space)
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
		XMFLOAT3A point = position;
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
void Transform::SetPosition(const XMFLOAT4A &position, ReferenceSpace space)
{
	SetPosition(To3(position), space);
}
void Transform::SetRotation(const XMFLOAT4A &rotation, ReferenceSpace space)
{
	if (!_parent)
		space = Local;

	switch (space)
	{
	case Local:
		_localRotation = rotation;
		break;

	case World:
		Store(_localRotation, XMQuaternionMultiply(XMQuaternionInverse(Load(_parent->WorldRotation())), Load(rotation)));
		break;

	default:
		ErrMsg("Invalid reference space!");
		break;
	}

	SetWorldRotationDirty();
	_isLocalMatrixDirty = true;
}
void Transform::SetScale(const XMFLOAT3A &scale, ReferenceSpace space)
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
		const XMVECTOR worldRotationQuat = Load(WorldRotation());

		XMFLOAT3A x = XMFLOAT3A(scale.x, 0, 0);
		XMFLOAT3A y = XMFLOAT3A(0, scale.y, 0);
		XMFLOAT3A z = XMFLOAT3A(0, 0, scale.z);

		Store(x, XMVector3Rotate(Load(x), worldRotationQuat));
		Store(x, XMVector3Rotate(Load(y), worldRotationQuat));
		Store(x, XMVector3Rotate(Load(z), worldRotationQuat));

		const XMMATRIX rsMat = XMMATRIX(
			x.x, x.y, x.z, 0,
			y.x, y.y, y.z, 0,
			z.x, z.y, z.z, 0,
			0,   0,   0,   1
		);

		_localScale = XMFLOAT3A(1, 1, 1);

		const XMMATRIX inverseRS = XMMatrixInverse(nullptr, Load(GetWorldRotationAndScale()));
		XMFLOAT4X4A localRS;
		Store(localRS, XMMatrixMultiply(inverseRS, rsMat));

		// Main diagonal is the new scale
		_localScale = XMFLOAT3A(localRS._11, localRS._22, localRS._33);
		break;
	}

	default:
		ErrMsg("Invalid reference space!");
		break;
	}

	SetWorldScaleDirty();
	_isLocalMatrixDirty = true;
}
void Transform::SetScale(const XMFLOAT4A &scale, ReferenceSpace space)
{
	SetScale(To3(scale), space);
}
void Transform::SetMatrix(const XMFLOAT4X4A &mat, ReferenceSpace space)
{
	if (!_parent)
		space = Local;

	switch (space)
	{
	case Local:
		XMVECTOR s, r, p;

		XMMatrixDecompose(&s, &r, &p, Load(mat));

		Store(_localScale, s);
		Store(_localRotation, r);
		Store(_localPosition, p);
		break;

	case World:
	{
		XMVECTOR s, r, p;
		XMMatrixDecompose(&s, &r, &p, Load(mat));
		XMFLOAT3A scale, pos;
		XMFLOAT4A rot;

		Store(scale, s);
		Store(rot, r);
		Store(pos, p);

		SetPosition(pos, World);
		SetRotation(rot, World);
		SetScale(scale, World);
		break;
	}

	default:
		ErrMsg("Invalid reference space!");
		break;
	}

	SetWorldRotationDirty();
	_isWorldPositionDirty = true;
	_isWorldScaleDirty = true;
	_isLocalMatrixDirty = true;
}

void Transform::Move(const XMFLOAT3A &direction, ReferenceSpace space)
{
	switch (space)
	{
	case Local:
		Store(_localPosition, XMVectorAdd(Load(_localPosition), Load(direction)));
		break;

	case World:
	{
		XMFLOAT3A newPos;
		Store(newPos, XMVectorAdd(Load(WorldPosition()), Load(direction)));
		SetPosition(newPos, World);
		break;
	}

	default:
		ErrMsg("Invalid reference space!");
		break;
	}

	SetWorldPositionDirty();
	_isLocalMatrixDirty = true;
}
void Transform::Move(const XMFLOAT4A &direction, ReferenceSpace space)
{
	Move(To3(direction), space);

}
void Transform::Rotate(const XMFLOAT3A &euler, ReferenceSpace space)
{
	switch (space)
	{
	case Local:
		Store(_localRotation, XMQuaternionMultiply(Load(_localRotation), XMQuaternionRotationRollPitchYaw(euler.x, euler.y, euler.z)));
		break;

	case World:
	{
		XMFLOAT4A newRot;
		Store(newRot, XMQuaternionMultiply(Load(WorldRotation()), XMQuaternionRotationRollPitchYaw(euler.x, euler.y, euler.z)));
		SetRotation(newRot, World);
		break;
	}

	default:
		ErrMsg("Invalid reference space!");
		break;
	}

	SetWorldRotationDirty();
	_isLocalMatrixDirty = true;
}
void Transform::Rotate(const XMFLOAT4A &euler, ReferenceSpace space)
{
	Rotate(To3(euler), space);
}
void Transform::Scale(const XMFLOAT3A &scale, ReferenceSpace space)
{
	switch (space)
	{
	case Local:
		Store(_localScale, XMVectorAdd(Load(_localScale), Load(scale)));
		break;

	case World:
	{
		XMFLOAT3A newScale;
		Store(newScale, XMVectorAdd(Load(WorldScale()), Load(scale)));
		SetPosition(newScale, World);
		break;
	}

	default:
		ErrMsg("Invalid reference space!");
		break;
	}

	SetWorldScaleDirty();
	_isLocalMatrixDirty = true;
}
void Transform::Scale(const XMFLOAT4A &scale, ReferenceSpace space)
{
	Scale(To3(scale), space);
}

void Transform::MoveRelative(const XMFLOAT3A &direction, ReferenceSpace space)
{
	XMFLOAT3A right, up, forward;
	GetAxes(&right, &up, &forward, space);

	XMFLOAT3A relativeDirection = {
		(right.x * direction.x + up.x * direction.y + forward.x * direction.z),
		(right.y * direction.x + up.y * direction.y + forward.y * direction.z),
		(right.z * direction.x + up.z * direction.y + forward.z * direction.z)
	};

	Move(relativeDirection, space);
}
void Transform::MoveRelative(const XMFLOAT4A &direction, ReferenceSpace space)
{
	MoveRelative(To3(direction), space);

}
void Transform::RotateAxis(const DirectX::XMFLOAT3A &axis, const float &amount, ReferenceSpace space)
{
	XMVECTOR axisVec = Load(axis);
	XMVECTOR currentRotation = Load(space == World ? *WorldRotation() : _localRotation);

	XMFLOAT4A newRotation;
	Store(newRotation, XMQuaternionRotationAxis(axisVec, amount));

	SetRotation(newRotation, space);
}
void Transform::RotateAxis(const DirectX::XMFLOAT4A &axis, const float &amount, ReferenceSpace space)
{
	RotateAxis(To3(axis), amount, space);
}

const XMFLOAT3A Transform::GetEuler(ReferenceSpace space)
{

	XMFLOAT4A rot = GetRotation(space);

	const float xx = rot.x * rot.x;
	const float yy = rot.y * rot.y;
	const float zz = rot.z * rot.z;

	const float m31 = 2.f * rot.x * rot.z + 2.f * rot.y * rot.w;
	const float m32 = 2.f * rot.y * rot.z - 2.f * rot.x * rot.w;
	const float m33 = 1.f - 2.f * xx - 2.f * yy;

	const float cy = sqrtf(m33 * m33 + m31 * m31);
	const float cx = atan2f(-m32, cy);
	if (cy > 16.f * FLT_EPSILON)
	{
		const float m12 = 2.f * rot.x * rot.y + 2.f * rot.z * rot.w;
		const float m22 = 1.f - 2.f * xx - 2.f * zz;

		return XMFLOAT3A(cx, atan2f(m31, m33), atan2f(m12, m22));
	}
	else
	{
		const float m11 = 1.f - 2.f * yy - 2.f * zz;
		const float m21 = 2.f * rot.x * rot.y - 2.f * rot.z * rot.w;

		return XMFLOAT3A(cx, 0.f, atan2f(-m21, m11));
	}
}
void Transform::SetEuler(const XMFLOAT3A &pitchYawRoll, ReferenceSpace space)
{
	XMFLOAT4A newRot;
	Store(newRot, XMQuaternionRotationRollPitchYaw(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z));
	SetRotation(newRot, space);
}
void Transform::SetEuler(const DirectX::XMFLOAT4A &rollPitchYaw, ReferenceSpace space)
{
	SetEuler(To3(rollPitchYaw), space);
}

bool Transform::UpdateConstantBuffer(ID3D11DeviceContext *context)
{
	const XMMATRIX transposeWorldMatrix = XMMatrixTranspose(Load(*WorldMatrix()));
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

const XMFLOAT4X4A &Transform::GetLocalMatrix()
{
	return *LocalMatrix();
}
const XMFLOAT4X4A &Transform::GetWorldMatrix()
{
	return *WorldMatrix();
}
