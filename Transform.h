#pragma once

#include <vector>
#include <memory>
#include <DirectXMath.h>
#include <SimpleMath.h>

#include "ConstantBufferD3D11.h"


class Transform
{
private:
	DirectX::XMFLOAT4A
		_right		= { 1.0f, 0.0f, 0.0f, 0.0f },
		_up			= { 0.0f, 1.0f, 0.0f, 0.0f },
		_forward	= { 0.0f, 0.0f, 1.0f, 0.0f },
		_pos		= { 0.0f, 0.0f, 0.0f, 1.0f },
		_scale		= { 1.0f, 1.0f, 1.0f, 0.0f };

	ConstantBufferD3D11 _worldMatrixBuffer;
	bool _isDirty = true;

	Transform *_parent = nullptr;
	std::vector<Transform*> _children;

	void NormalizeBases();
	void OrthogonalizeBases();

	void AddChild(Transform *child);
	void RemoveChild(Transform *child);

public:
	Transform() = default;
	explicit Transform(ID3D11Device *device, const DirectX::XMMATRIX &worldMatrix = DirectX::XMMatrixIdentity());
	~Transform();
	Transform(const Transform &other) = delete;
	Transform &operator=(const Transform &other) = delete;
	Transform(Transform &&other) = delete;
	Transform &operator=(Transform &&other) = delete;


	[[nodiscard]] bool Initialize(ID3D11Device *device);
	[[nodiscard]] bool Initialize(ID3D11Device *device, const DirectX::XMMATRIX& worldMatrix);

	void SetParent(Transform *parent, bool keepWorldTransform = false);
	[[nodiscard]] Transform *GetParent() const;

	void Move(const DirectX::XMFLOAT4A &movement);
	void Rotate(const DirectX::XMFLOAT4A &rotation);
	void ScaleAbsolute(const DirectX::XMFLOAT4A &scale);

	void MoveLocal(const DirectX::XMFLOAT4A &movement);
	void RotateLocal(const DirectX::XMFLOAT4A &rotation);
	void ScaleRelative(const DirectX::XMFLOAT4A &scale);

	void RotateByQuaternion(const DirectX::XMVECTOR &quaternion);

	void SetPosition(const DirectX::XMFLOAT4A &position);
	void SetScale(const DirectX::XMFLOAT4A &scale);
	void SetAxes(const DirectX::XMFLOAT4A &right, const DirectX::XMFLOAT4A &up, const DirectX::XMFLOAT4A &forward);

	[[nodiscard]] const DirectX::XMFLOAT4A &GetPosition() const;
	[[nodiscard]] const DirectX::XMFLOAT4A &GetScale() const;
	[[nodiscard]] const DirectX::XMFLOAT4A &GetRight() const;
	[[nodiscard]] const DirectX::XMFLOAT4A &GetUp() const;
	[[nodiscard]] const DirectX::XMFLOAT4A &GetForward() const;

	void SetDirty();
	[[nodiscard]] bool GetDirty() const;

	[[nodiscard]] bool UpdateConstantBuffer(ID3D11DeviceContext *context);
	[[nodiscard]] ID3D11Buffer *GetConstantBuffer() const;
	[[nodiscard]] DirectX::XMMATRIX GetLocalMatrix() const;
	[[nodiscard]] DirectX::XMMATRIX GetWorldMatrix() const;
};
