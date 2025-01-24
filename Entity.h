#pragma once

#include "Transform.h"
#include "Time.h"
#include "Input.h"
#include "Graphics.h"


enum class EntityType
{
	OBJECT,
	EMITTER,
};


class Entity
{
private:
	UINT _entityID;
	std::string _name = "";


protected:
	bool _isInitialized = false;
	Transform _transform;

	DirectX::BoundingOrientedBox _bounds;
	DirectX::BoundingOrientedBox _transformedBounds;
	bool _recalculateBounds = true;

	Entity *_parent = nullptr;
	std::vector<Entity *> _children;

	Entity(UINT id, const DirectX::BoundingOrientedBox &bounds);
	[[nodiscard]] bool Initialize(ID3D11Device *device, const std::string &name);

	inline void AddChild(Entity *child, bool keepWorldTransform = false);
	inline void RemoveChild(Entity *child, bool keepWorldTransform = false);

	[[nodiscard]] bool InternalUpdate(ID3D11DeviceContext *context);
	[[nodiscard]] bool InternalBindBuffers(ID3D11DeviceContext *context) const;
	[[nodiscard]] bool InternalRender(CameraD3D11 *camera);

public:
	virtual ~Entity();
	Entity(const Entity &other) = delete;
	Entity &operator=(const Entity &other) = delete;
	Entity(Entity &&other) = delete;
	Entity &operator=(Entity &&other) = delete;

	[[nodiscard]] bool IsInitialized() const;

	void SetDirty();

	void SetParent(Entity *parent, bool keepWorldTransform = false);
	[[nodiscard]] Entity *GetParent();
	[[nodiscard]] const std::vector<Entity *> *GetChildren();

	void SetName(const std::string &name);
	[[nodiscard]] const std::string GetName() const;

	[[nodiscard]] UINT GetID() const;
	[[nodiscard]] Transform *GetTransform();
	[[nodiscard]] virtual EntityType GetType() const = 0;

	void StoreBounds(DirectX::BoundingOrientedBox &entityBounds);

	[[nodiscard]] virtual bool Update(ID3D11DeviceContext *context, Time &time, const Input &input) = 0;
	[[nodiscard]] virtual bool BindBuffers(ID3D11DeviceContext *context) const = 0;
	[[nodiscard]] virtual bool Render(CameraD3D11 *camera) = 0;
};
