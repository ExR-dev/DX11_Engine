#include "Transform.h"

using namespace DirectX;


void Transform::UpdateMatrices()
{
	// TODO
}

void Transform::UpdateVectors()
{
	// TODO
}


Transform::Transform()
{
	// TODO
}

Transform::~Transform()
{
	// TODO
}


void Transform::Move(const XMFLOAT3 &movement)
{
	// TODO
}

void Transform::Rotate(const XMFLOAT3 &rotation)
{
	// TODO
}

void Transform::Scale(const XMFLOAT3 &factor)
{
	// TODO
}


void Transform::SetPosition(const XMFLOAT3 &position)
{
	// TODO
}

void Transform::SetRotation(const XMFLOAT3 &rotation)
{
	// TODO
}

void Transform::SetScale(const XMFLOAT3 &scale)
{
	// TODO
}


XMFLOAT3 Transform::GetRight() const
{
	// TODO
	return {};
}

XMFLOAT3 Transform::GetUp() const
{
	// TODO
	return {};
}

XMFLOAT3 Transform::GetForward() const
{
	// TODO
	return {};
}


XMFLOAT3 Transform::GetPosition() const
{
	// TODO
	return {};
}

XMFLOAT3 Transform::GetRotation() const
{
	// TODO
	return {};
}

XMFLOAT3 Transform::GetScale() const
{
	// TODO
	return {};
}


void Transform::SetWorldMatrix(const XMMATRIX &matrix)
{
	// TODO
}

XMMATRIX Transform::GetWorldMatrix() const
{
	// TODO
	return {};
}
