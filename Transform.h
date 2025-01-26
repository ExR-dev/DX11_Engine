#pragma once

#include <vector>
#include <memory>
#include <DirectXMath.h>

#include "ConstantBufferD3D11.h"


enum ReferenceSpace
{
	Local,
	World
};

class Transform
{
private:
	Transform *_parent = nullptr;
	std::vector<Transform*> _children;

	DirectX::XMFLOAT3A _localPosition = { 0, 0, 0 };
	DirectX::XMFLOAT4A _localRotation = { 0, 0, 0, 1 };
	DirectX::XMFLOAT3A _localScale =	{ 1, 1, 1 };
	DirectX::XMFLOAT4X4A _localMatrix = { 1, 0, 0, 0,
										  0, 1, 0, 0,
										  0, 0, 1, 0,
										  0, 0, 0, 1, };

	DirectX::XMFLOAT3A _worldPosition = { 0, 0, 0 };
	DirectX::XMFLOAT4A _worldRotation = { 0, 0, 0, 1 };
	DirectX::XMFLOAT3A _worldScale =	{ 1, 1, 1 };
	DirectX::XMFLOAT4X4A _worldMatrix = { 1, 0, 0, 0, 
										  0, 1, 0, 0, 
										  0, 0, 1, 0, 
										  0, 0, 0, 1, };

	ConstantBufferD3D11 _worldMatrixBuffer;

	bool _isDirty = true;
	bool _isWorldPositionDirty = true;	// Dirtied by parent position, rotation and scale.
	bool _isWorldRotationDirty = true;	// Dirtied by parent rotation.
	bool _isWorldScaleDirty = true;		// Dirtied by parent rotation and scale.
	bool _isWorldMatrixDirty = true;	// Dirtied by parent position, rotation and scale.
	bool _isLocalMatrixDirty = true;	// Never dirtied by parent.

	const DirectX::XMFLOAT4A To4(const DirectX::XMFLOAT3A &vec) const;
	const DirectX::XMFLOAT3A To3(const DirectX::XMFLOAT4A &vec) const;

	[[nodiscard]] inline DirectX::XMVECTOR Load(const DirectX::XMFLOAT3A &float3A) const;
	[[nodiscard]] inline DirectX::XMVECTOR Load(const DirectX::XMFLOAT4A &float4A) const;
	[[nodiscard]] inline DirectX::XMMATRIX Load(const DirectX::XMFLOAT4X4A &float4x4A) const;
	[[nodiscard]] inline DirectX::XMVECTOR Load(const DirectX::XMFLOAT3A *float3A) const;
	[[nodiscard]] inline DirectX::XMVECTOR Load(const DirectX::XMFLOAT4A *float4A) const;
	[[nodiscard]] inline DirectX::XMMATRIX Load(const DirectX::XMFLOAT4X4A *float4x4A) const;
	inline void Store(DirectX::XMFLOAT3A &dest, const DirectX::XMVECTOR &vec) const;
	inline void Store(DirectX::XMFLOAT4A &dest, const DirectX::XMVECTOR &vec) const;
	inline void Store(DirectX::XMFLOAT4X4A &dest, const DirectX::XMMATRIX &mat) const;

	inline void AddChild(Transform *child);
	inline void RemoveChild(Transform *child);

	void SetWorldPositionDirty();
	void SetWorldRotationDirty();
	void SetWorldScaleDirty();
	void SetAllDirty();

	[[nodiscard]] DirectX::XMFLOAT3A *WorldPosition();
	[[nodiscard]] DirectX::XMFLOAT4A *WorldRotation();
	[[nodiscard]] DirectX::XMFLOAT3A *WorldScale();
	[[nodiscard]] DirectX::XMFLOAT4X4A *WorldMatrix();
	[[nodiscard]] DirectX::XMFLOAT4X4A *LocalMatrix();

	[[nodiscard]] const DirectX::XMFLOAT3A InverseTransformPoint(DirectX::XMFLOAT3A &point) const;
	[[nodiscard]] const DirectX::XMFLOAT4X4A GetWorldRotationAndScale();

public:
	Transform() = default;
	explicit Transform(ID3D11Device *device, const DirectX::XMFLOAT4X4A &worldMatrix);
	~Transform();
	Transform(const Transform &other) = delete;
	Transform &operator=(const Transform &other) = delete;
	Transform(Transform &&other) = delete;
	Transform &operator=(Transform &&other) = delete;

	[[nodiscard]] bool Initialize(ID3D11Device *device);
	[[nodiscard]] bool Initialize(ID3D11Device *device, const DirectX::XMFLOAT4X4A &worldMatrix);

	void SetDirty();
	[[nodiscard]] bool IsDirty() const;

	void SetParent(Transform *parent, bool worldPositionStays = false);
	[[nodiscard]] Transform *GetParent() const;

	[[nodiscard]] const DirectX::XMFLOAT3A &Right(ReferenceSpace space = World);
	[[nodiscard]] const DirectX::XMFLOAT3A &Up(ReferenceSpace space = World);
	[[nodiscard]] const DirectX::XMFLOAT3A &Forward(ReferenceSpace space = World);

	[[nodiscard]] const DirectX::XMFLOAT3A &GetPosition(ReferenceSpace space = World);
	[[nodiscard]] const DirectX::XMFLOAT4A &GetRotation(ReferenceSpace space = World);
	[[nodiscard]] const DirectX::XMFLOAT3A &GetScale(ReferenceSpace space = Local);

	void SetPosition(const DirectX::XMFLOAT3A &position, ReferenceSpace space = World);
	void SetPosition(const DirectX::XMFLOAT4A &position, ReferenceSpace space = World);
	void SetRotation(const DirectX::XMFLOAT4A &rotation, ReferenceSpace space = World);
	void SetScale(const DirectX::XMFLOAT3A &scale, ReferenceSpace space = Local);
	void SetScale(const DirectX::XMFLOAT4A &scale, ReferenceSpace space = Local);

	void Move(const DirectX::XMFLOAT3A &direction, ReferenceSpace space = World);
	void Move(const DirectX::XMFLOAT4A &direction, ReferenceSpace space = World);
	void Rotate(const DirectX::XMFLOAT3A &euler, ReferenceSpace space = World);
	void Rotate(const DirectX::XMFLOAT4A &euler, ReferenceSpace space = World);
	void Scale(const DirectX::XMFLOAT3A &scale, ReferenceSpace space = Local);
	void Scale(const DirectX::XMFLOAT4A &scale, ReferenceSpace space = Local);

	[[nodiscard]] const DirectX::XMFLOAT3A GetEuler(ReferenceSpace space = World);
	void SetEuler(const DirectX::XMFLOAT3A &rollPitchYaw, ReferenceSpace space = World);
	void SetEuler(const DirectX::XMFLOAT4A &rollPitchYaw, ReferenceSpace space = World);

	[[nodiscard]] bool UpdateConstantBuffer(ID3D11DeviceContext *context);
	[[nodiscard]] ID3D11Buffer *GetConstantBuffer() const;
	[[nodiscard]] const DirectX::XMFLOAT4X4A &GetLocalMatrix();
	[[nodiscard]] const DirectX::XMFLOAT4X4A &GetWorldMatrix();
};