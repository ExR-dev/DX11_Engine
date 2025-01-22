#pragma once
/*
#include <DirectXMath.h>
#include <SimpleMath.h>


class SimpleTransform
{
private:
	DirectX::SimpleMath::Vector3
		m_right = { 1.0f, 0.0f, 0.0f },
		m_up = { 0.0f, 1.0f, 0.0f },
		m_forward = { 0.0f, 0.0f, 1.0f },
		m_position = { 0.0f, 0.0f, 0.0f },
		m_scale = { 1.0f, 1.0f, 1.0f };

	SimpleMath::Matrix m_worldMatrix;
	bool m_isDirty = true;

	SimpleTransform *_parent = nullptr;
	std::vector<SimpleTransform *> _children;

	void NormalizeBases();
	void OrthogonalizeBases();

	void AddChild(SimpleTransform *child);
	void RemoveChild(SimpleTransform *child);

public:
	SimpleTransform() = default;
	explicit SimpleTransform(const DirectX::SimpleMath::Matrix &worldMatrix);
	~SimpleTransform();
	SimpleTransform(const SimpleTransform &other) = delete;
	SimpleTransform &operator=(const SimpleTransform &other) = delete;
	SimpleTransform(SimpleTransform &&other) = delete;
	SimpleTransform &operator=(SimpleTransform &&other) = delete;

};*/