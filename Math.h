#pragma once

#include <DirectXMath.h>

/*struct Vector
{
private:
	DirectX::XMVECTOR _vector	= { 0.0f, 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT4A _float4	= { 0.0f, 0.0f, 0.0f, 0.0f };
	bool _isDirty = true;

public:
	Vector() = default;
	explicit Vector(DirectX::XMFLOAT3A vec);
	explicit Vector(DirectX::XMFLOAT4A vec);
	explicit Vector(DirectX::XMVECTOR vec);
	explicit Vector(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 0.0f);
	~Vector() = default;
	Vector(const Vector &other);
	Vector &operator=(const Vector &other) = delete;
	Vector(Vector &&other) = delete;
	Vector &operator=(Vector &&other) = delete;

	void Normalize();

	[[nodiscard]] const DirectX::XMFLOAT4A &GetPosition() const;


	[[nodiscard]] Vector G();
	[[nodiscard]] const Vector &GetScale() const;
	[[nodiscard]] const Vector &GetRight() const;
	[[nodiscard]] const Vector &GetUp() const;
	[[nodiscard]] const Vector &GetForward() const;

	[[nodiscard]] static const Vector Zero();
	[[nodiscard]] static const Vector One();
	[[nodiscard]] static const Vector Right();
	[[nodiscard]] static const Vector Up();
	[[nodiscard]] static const Vector Forward();
};*/