#pragma once

#include <DirectXMath.h>


class Transform
{
private:
	DirectX::XMMATRIX _world = DirectX::XMMatrixIdentity();

	DirectX::XMFLOAT3
		_pos = { 0.0f, 0.0f, 0.0f },
		_rot = { 0.0f, 0.0f, 0.0f },
		_size = { 1.0f, 1.0f, 1.0f };

	void UpdateMatrices();
	void UpdateVectors();

public:
	Transform();
	~Transform();


	void Move(const DirectX::XMFLOAT3 &movement);
	void Rotate(const DirectX::XMFLOAT3 &rotation);
	void Scale(const DirectX::XMFLOAT3 &factor);


	void SetPosition(const DirectX::XMFLOAT3 &position);
	void SetRotation(const DirectX::XMFLOAT3 &rotation);
	void SetScale(const DirectX::XMFLOAT3 &scale);


	DirectX::XMFLOAT3 GetRight() const;
	DirectX::XMFLOAT3 GetUp() const;
	DirectX::XMFLOAT3 GetForward() const;

	DirectX::XMFLOAT3 GetPosition() const;
	DirectX::XMFLOAT3 GetRotation() const;
	DirectX::XMFLOAT3 GetScale() const;


	void SetWorldMatrix(const DirectX::XMMATRIX &matrix);
	DirectX::XMMATRIX GetWorldMatrix() const;
};
