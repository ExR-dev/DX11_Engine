#pragma once

#include <vector>
#include <map>
#include <d3d11_4.h>
#include <DirectXCollision.h>
#include <DirectXMath.h>

#include "ConstantBufferD3D11.h"
#include "Content.h"
#include "Transform.h"


struct ProjectionInfo
{
	float fovAngleY = 80.0f * (DirectX::XM_PI / 180.0f);
	float aspectRatio = 1.0f;
	float nearZ = 0.1f;
	float farZ = 50.0f;
};

struct GeometryBufferData
{
	DirectX::XMFLOAT4X4A viewMatrix;
	DirectX::XMFLOAT4A position;
};


struct ResourceGroup
{
	UINT
		meshID = CONTENT_LOAD_ERROR,
		texID = CONTENT_LOAD_ERROR,
		normalID = CONTENT_LOAD_ERROR,
		specularID = CONTENT_LOAD_ERROR,
		reflectiveID = CONTENT_LOAD_ERROR,
		ambientID = CONTENT_LOAD_ERROR,
		heightID = CONTENT_LOAD_ERROR;

	bool operator<(const ResourceGroup &other) const
	{
		if (meshID != other.meshID)
			return meshID < other.meshID;

		if (texID != other.texID)
			return texID < other.texID;

		if (normalID != other.normalID)
			return normalID < other.normalID;

		if (specularID != other.specularID)
			return specularID < other.specularID;

		if (reflectiveID != other.reflectiveID)
			return reflectiveID < other.reflectiveID;

		if (ambientID != other.ambientID)
			return ambientID < other.ambientID;

		return heightID < other.heightID;
	}
};

struct RenderInstance
{
	void *subject;
	size_t subjectSize;
};


class CameraD3D11
{
private:
	Transform _transform;
	ProjectionInfo _defaultProjInfo, _currProjInfo;
	bool _ortho = false;

	union
	{
		DirectX::BoundingFrustum perspective = {};
		DirectX::BoundingOrientedBox ortho;
	} _bounds;
	union
	{
		DirectX::BoundingFrustum perspective = {};
		DirectX::BoundingOrientedBox ortho;
	} _transformedBounds;
	bool _recalculateBounds = true;

	ConstantBufferD3D11 _viewProjBuffer;
	ConstantBufferD3D11 *_viewProjPosBuffer = nullptr;
	ConstantBufferD3D11 *_posBuffer = nullptr;
	bool _isDirty = true;

	UINT _lastCullCount = 0;
	std::multimap<ResourceGroup, RenderInstance> _geometryRenderQueue; // Let batching be handled by multimap
	std::multimap<ResourceGroup, RenderInstance> _transparentRenderQueue;
	std::multimap<ResourceGroup, RenderInstance> _particleRenderQueue;


public:
	CameraD3D11() = default;
	CameraD3D11(ID3D11Device *device, const ProjectionInfo &projectionInfo,
		const DirectX::XMFLOAT4A &initialPosition = DirectX::XMFLOAT4A(0.0f, 0.0f, 0.0f, 0.0f), 
		bool hasCSBuffer = true, bool isOrtho = false);
	~CameraD3D11();
	CameraD3D11(const CameraD3D11 &other) = delete;
	CameraD3D11 &operator=(const CameraD3D11 &other) = delete;
	CameraD3D11(CameraD3D11 &&other) = delete;
	CameraD3D11 &operator=(CameraD3D11 &&other) = delete;

	[[nodiscard]] bool Initialize(ID3D11Device *device, const ProjectionInfo &projectionInfo,
		const DirectX::XMFLOAT4A &initialPosition = DirectX::XMFLOAT4A(0.0f, 0.0f, 0.0f, 0.0f), bool hasCSBuffer = true, bool isOrtho = false);

	void Move(float amount, const DirectX::XMFLOAT4A &direction);
	void MoveLocal(float amount, const DirectX::XMFLOAT4A &direction);

	void MoveForward(float amount);
	void MoveRight(float amount);
	void MoveUp(float amount);

	void RotateRoll(float amount);
	void RotatePitch(float amount);
	void RotateYaw(float amount);

	void LookX(float amount);
	void LookY(float amount);

	void SetFOV(float amount);
	void SetOrtho(bool state);

	[[nodiscard]] const DirectX::XMFLOAT4A &GetPosition() const;
	[[nodiscard]] const DirectX::XMFLOAT4A &GetForward() const;
	[[nodiscard]] const DirectX::XMFLOAT4A &GetRight() const;
	[[nodiscard]] const DirectX::XMFLOAT4A &GetUp() const;

	[[nodiscard]] DirectX::XMFLOAT4X4A GetViewMatrix() const;
	[[nodiscard]] DirectX::XMFLOAT4X4A GetProjectionMatrix() const;
	[[nodiscard]] DirectX::XMFLOAT4X4A GetViewProjectionMatrix() const;
	[[nodiscard]] const ProjectionInfo &GetCurrProjectionInfo() const;

	[[nodiscard]] bool ScaleToContents(const std::vector<DirectX::XMFLOAT4A> &nearBounds, const std::vector<DirectX::XMFLOAT4A> &innerBounds);
	[[nodiscard]] bool FitPlanesToPoints(const std::vector<DirectX::XMFLOAT4A> &points);
	[[nodiscard]] bool UpdateBuffers(ID3D11DeviceContext *context);

	[[nodiscard]] bool BindShadowCasterBuffers(ID3D11DeviceContext *context) const;
	[[nodiscard]] bool BindGeometryBuffers(ID3D11DeviceContext *context) const;
	[[nodiscard]] bool BindLightingBuffers(ID3D11DeviceContext *context) const;
	[[nodiscard]] bool BindTransparentBuffers(ID3D11DeviceContext *context) const;
	[[nodiscard]] bool BindViewBuffers(ID3D11DeviceContext *context) const;
	[[nodiscard]] bool BindMainBuffers(ID3D11DeviceContext *context) const;

	[[nodiscard]] bool StoreBounds(DirectX::BoundingFrustum &bounds);
	[[nodiscard]] bool StoreBounds(DirectX::BoundingOrientedBox &bounds);

	void QueueGeometry(const ResourceGroup &resources, const RenderInstance &instance);
	void QueueTransparent(const ResourceGroup &resources, const RenderInstance &instance);
	void QueueEmitter(const ResourceGroup &resources, const RenderInstance &instance);
	void ResetRenderQueue();

	[[nodiscard]] UINT GetCullCount() const;
	[[nodiscard]] const std::multimap<ResourceGroup, RenderInstance> &GetGeometryQueue() const;
	[[nodiscard]] const std::multimap<ResourceGroup, RenderInstance> &GetTransparentQueue() const;
	[[nodiscard]] const std::multimap<ResourceGroup, RenderInstance> &GetParticleQueue() const;

	[[nodiscard]] bool GetOrtho() const;
	[[nodiscard]] const Transform &GetTransform() const;
	[[nodiscard]] ID3D11Buffer *GetCameraVSBuffer() const;
	[[nodiscard]] ID3D11Buffer *GetCameraGSBuffer() const;
	[[nodiscard]] ID3D11Buffer *GetCameraCSBuffer() const;
};