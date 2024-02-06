#pragma once

#include <DirectXMath.h>

using namespace DirectX;


class Transform
{
private:
	XMMATRIX _world = XMMatrixIdentity();

	XMFLOAT3
		_pos = { 0.0f, 0.0f, 0.0f },
		_rot = { 0.0f, 0.0f, 0.0f },
		_size = { 1.0f, 1.0f, 1.0f };

	void UpdateMatrices();
	void UpdateVectors();

public:
	Transform();
	Transform(const XMMATRIX &worldMatrix);
	~Transform();


	void Move(const XMFLOAT3 &movement);
	void Rotate(const XMFLOAT3 &rotation);
	void Scale(const XMFLOAT3 &factor);


	void SetPosition(const XMFLOAT3 &position);
	void SetRotation(const XMFLOAT3 &rotation);
	void SetScale(const XMFLOAT3 &scale);


	XMFLOAT3 GetRight() const;
	XMFLOAT3 GetUp() const;
	XMFLOAT3 GetForward() const;

	XMFLOAT3 GetPosition() const;
	XMFLOAT3 GetRotation() const;
	XMFLOAT3 GetScale() const;


	void SetWorldMatrix(const XMMATRIX &matrix);
	XMMATRIX GetWorldMatrix() const;
};
