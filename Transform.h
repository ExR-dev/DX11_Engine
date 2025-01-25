#pragma once

#include <vector>
#include <memory>
#include <DirectXMath.h>
#include <SimpleMath.h>

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

	DirectX::SimpleMath::Vector3 _localPosition = DirectX::SimpleMath::Vector3::Zero;
	DirectX::SimpleMath::Quaternion _localRotation = DirectX::SimpleMath::Quaternion::Identity;
	DirectX::SimpleMath::Vector3 _localScale = DirectX::SimpleMath::Vector3::One;
	DirectX::SimpleMath::Matrix _localMatrix = DirectX::SimpleMath::Matrix::Identity;

	DirectX::SimpleMath::Vector3 _worldPosition = DirectX::SimpleMath::Vector3::Zero;
	DirectX::SimpleMath::Quaternion _worldRotation = DirectX::SimpleMath::Quaternion::Identity;
	DirectX::SimpleMath::Vector3 _worldScale = DirectX::SimpleMath::Vector3::One;
	DirectX::SimpleMath::Matrix _worldMatrix = DirectX::SimpleMath::Matrix::Identity;

	ConstantBufferD3D11 _worldMatrixBuffer;

	bool _isDirty = true;
	bool _isWorldPositionDirty = true;	// Dirtied by parent position, rotation and scale.
	bool _isWorldRotationDirty = true;	// Dirtied by parent rotation.
	bool _isWorldScaleDirty = true;		// Dirtied by parent rotation and scale.
	bool _isWorldMatrixDirty = true;	// Dirtied by parent position, rotation and scale.
	bool _isLocalMatrixDirty = true;	// Never dirtied by parent.


	inline void AddChild(Transform *child);
	inline void RemoveChild(Transform *child);

	void SetWorldPositionDirty();
	void SetWorldRotationDirty();
	void SetWorldScaleDirty();
	void SetAllDirty();

	[[nodiscard]] DirectX::SimpleMath::Vector3 *WorldPosition();
	[[nodiscard]] DirectX::SimpleMath::Quaternion *WorldRotation();
	[[nodiscard]] DirectX::SimpleMath::Vector3 *WorldScale();
	[[nodiscard]] DirectX::SimpleMath::Matrix *WorldMatrix();
	[[nodiscard]] DirectX::SimpleMath::Matrix *LocalMatrix();

	[[nodiscard]] const DirectX::SimpleMath::Vector3 InverseTransformPoint(DirectX::SimpleMath::Vector3 &point) const;
	[[nodiscard]] const DirectX::SimpleMath::Matrix GetGlobalRotationAndScale();

public:
	Transform() = default;
	explicit Transform(ID3D11Device *device, const DirectX::SimpleMath::Matrix &worldMatrix = DirectX::SimpleMath::Matrix::Identity);
	~Transform();
	Transform(const Transform &other) = delete;
	Transform &operator=(const Transform &other) = delete;
	Transform(Transform &&other) = delete;
	Transform &operator=(Transform &&other) = delete;

	[[nodiscard]] bool Initialize(ID3D11Device *device);
	[[nodiscard]] bool Initialize(ID3D11Device *device, const DirectX::SimpleMath::Matrix &worldMatrix);

	void SetDirty();
	[[nodiscard]] bool IsDirty() const;

	void SetParent(Transform *parent, bool worldPositionStays = false);
	[[nodiscard]] Transform *GetParent() const;

	[[nodiscard]] const DirectX::SimpleMath::Vector3 Right(ReferenceSpace space = World);
	[[nodiscard]] const DirectX::SimpleMath::Vector3 Up(ReferenceSpace space = World);
	[[nodiscard]] const DirectX::SimpleMath::Vector3 Forward(ReferenceSpace space = World);

	[[nodiscard]] const DirectX::SimpleMath::Vector3 &GetPosition(ReferenceSpace space = World);
	[[nodiscard]] const DirectX::SimpleMath::Quaternion &GetRotation(ReferenceSpace space = World);
	[[nodiscard]] const DirectX::SimpleMath::Vector3 &GetScale(ReferenceSpace space = Local);

	void SetPosition(const DirectX::SimpleMath::Vector3 &position, ReferenceSpace space = World);
	void SetRotation(const DirectX::SimpleMath::Quaternion &rotation, ReferenceSpace space = World);
	void SetScale(const DirectX::SimpleMath::Vector3 &scale, ReferenceSpace space = Local);

	void Move(const DirectX::SimpleMath::Vector3 &direction, ReferenceSpace space = World);
	void Rotate(const DirectX::SimpleMath::Vector3 &euler, ReferenceSpace space = World);

	[[nodiscard]] const DirectX::SimpleMath::Vector3 GetEuler(ReferenceSpace space = World);
	void SetEuler(const DirectX::SimpleMath::Vector3 &rollPitchYaw, ReferenceSpace space = World);

	[[nodiscard]] bool UpdateConstantBuffer(ID3D11DeviceContext *context);
	[[nodiscard]] ID3D11Buffer *GetConstantBuffer() const;
	[[nodiscard]] DirectX::SimpleMath::Matrix GetLocalMatrix();
	[[nodiscard]] DirectX::SimpleMath::Matrix GetWorldMatrix();
};