#include "CameraD3D11.h"

#include <algorithm>

#include "ErrMsg.h"


#define TO_VEC(x)		( *reinterpret_cast<XMVECTOR *>(&(x))		)
#define TO_CONST_VEC(x) ( *reinterpret_cast<const XMVECTOR *>(&(x))	)


void CameraD3D11::Move(const float amount, const XMFLOAT4A &direction)
{
	_transform.Move({
		direction.x * amount,
		direction.y * amount,
		direction.z * amount,
		0.0f
	});
	_isDirty = true;
	_recalculateFrustum = true;
}

void CameraD3D11::MoveLocal(const float amount, const XMFLOAT4A &direction)
{
	_transform.MoveLocal({ 
		direction.x * amount,
		direction.y * amount,
		direction.z * amount,
		0.0f
	});
	_isDirty = true;
	_recalculateFrustum = true;
}


CameraD3D11::CameraD3D11(ID3D11Device *device, const ProjectionInfo &projectionInfo, const XMFLOAT4A &initialPosition, const bool hasCSBuffer)
{
	if (!Initialize(device, projectionInfo, initialPosition, hasCSBuffer))
		ErrMsg("Failed to initialize camera buffer in camera constructor!");
}

CameraD3D11::~CameraD3D11()
{
	delete _cameraGSBuffer;

	delete _cameraCSBuffer;
}


bool CameraD3D11::Initialize(ID3D11Device *device, const ProjectionInfo &projectionInfo, const XMFLOAT4A &initialPosition, const bool hasCSBuffer)
{
	_currProjInfo = _defaultProjInfo = projectionInfo;
	_transform.SetPosition(initialPosition);

	const XMFLOAT4X4A viewProjMatrix = GetViewProjectionMatrix();
	if (!_cameraVSBuffer.Initialize(device, sizeof(XMFLOAT4X4A), &viewProjMatrix))
	{
		ErrMsg("Failed to initialize camera VS buffer!");
		return false;
	}

	if (hasCSBuffer)
	{
		const GeometryBufferData bufferData = { GetViewMatrix(), _transform.GetPosition() };
		_cameraGSBuffer = new ConstantBufferD3D11();
		if (!_cameraGSBuffer->Initialize(device, sizeof(GeometryBufferData), &bufferData))
		{
			ErrMsg("Failed to initialize camera GS buffer!");
			return false;
		}

		_cameraCSBuffer = new ConstantBufferD3D11();
		if (!_cameraCSBuffer->Initialize(device, sizeof(XMFLOAT4A), &initialPosition))
		{
			ErrMsg("Failed to initialize camera CS buffer!");
			return false;
		}
	}

	XMFLOAT4X4A projMatrix = GetProjectionMatrix();
	DirectX::BoundingFrustum::CreateFromMatrix(_frustum, *reinterpret_cast<XMMATRIX *>(&projMatrix));

	return true;
}


void CameraD3D11::MoveForward(const float amount)
{
	MoveLocal(amount, { 0, 0, 1, 0 });
	_isDirty = true;
	_recalculateFrustum = true;
}

void CameraD3D11::MoveRight(const float amount)
{
	MoveLocal(amount, { 1, 0, 0, 0 });
	_isDirty = true;
	_recalculateFrustum = true;
}

void CameraD3D11::MoveUp(const float amount)
{
	MoveLocal(amount, { 0, 1, 0, 0 });
	_isDirty = true;
	_recalculateFrustum = true;
}


void CameraD3D11::RotateRoll(const float amount)
{
	_transform.RotateLocal({ 0, 0, amount, 0 });
	_isDirty = true;
	_recalculateFrustum = true;
}

void CameraD3D11::RotatePitch(const float amount)
{
	_transform.RotateLocal({ amount, 0, 0, 0 });
	_isDirty = true;
	_recalculateFrustum = true;
}

void CameraD3D11::RotateYaw(const float amount)
{
	_transform.RotateLocal({ 0, amount, 0, 0 });
	_isDirty = true;
	_recalculateFrustum = true;
}


void CameraD3D11::LookX(const float amount)
{
	_transform.Rotate({ 0, amount, 0, 0 });
	_isDirty = true;
	_recalculateFrustum = true;
}

void CameraD3D11::LookY(const float amount)
{
	_transform.RotateLocal({ amount, 0, 0, 0 });
	_isDirty = true;
	_recalculateFrustum = true;
}


const XMFLOAT4A &CameraD3D11::GetRight() const		{ return _transform.GetRight();		}
const XMFLOAT4A &CameraD3D11::GetUp() const			{ return _transform.GetUp();		}
const XMFLOAT4A &CameraD3D11::GetForward() const	{ return _transform.GetForward();	}
const XMFLOAT4A &CameraD3D11::GetPosition() const	{ return _transform.GetPosition();	}


XMFLOAT4X4A CameraD3D11::GetViewMatrix() const
{
	const XMFLOAT4A
		cPos = _transform.GetPosition(),
		cFwd = _transform.GetForward(),
		cUp = _transform.GetUp();

	XMMATRIX projectionMatrix = XMMatrixLookAtLH(
		TO_CONST_VEC(cPos),
		TO_CONST_VEC(cPos) + TO_CONST_VEC(cFwd),
		TO_CONST_VEC(cUp)
	);

	return *reinterpret_cast<XMFLOAT4X4A *>(&projectionMatrix);
}

XMFLOAT4X4A CameraD3D11::GetProjectionMatrix() const
{
	XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(
		_currProjInfo.fovAngleY,
		_currProjInfo.aspectRatio,
		_currProjInfo.nearZ,
		_currProjInfo.farZ
	);

	return *reinterpret_cast<XMFLOAT4X4A *>(&projectionMatrix);
}

XMFLOAT4X4A CameraD3D11::GetViewProjectionMatrix() const
{
	XMFLOAT4A
		cPos = _transform.GetPosition(),
		cFwd = _transform.GetForward(),
		cUp = _transform.GetUp();

	XMFLOAT4X4A vpMatrix = { };

	XMStoreFloat4x4(
		&vpMatrix,
		XMMatrixTranspose(
			XMMatrixLookAtLH(
				TO_VEC(cPos),
				TO_VEC(cPos) + TO_VEC(cFwd),
				TO_VEC(cUp)
			) *
			XMMatrixPerspectiveFovLH(
				_currProjInfo.fovAngleY,
				_currProjInfo.aspectRatio,
				_currProjInfo.nearZ,
				_currProjInfo.farZ
			)
		)
	);

	return vpMatrix;
}

const ProjectionInfo &CameraD3D11::GetCurrProjectionInfo() const
{
	return _currProjInfo;
}


bool CameraD3D11::FitPlanesToPoints(const std::vector<XMFLOAT4A> &points)
{
	const float
		currNear = _currProjInfo.nearZ,
		currFar = _currProjInfo.farZ;

	const XMVECTOR origin = TO_CONST_VEC(_transform.GetPosition());
	const XMVECTOR direction = TO_CONST_VEC(_transform.GetForward());

	float minDist = FLT_MAX, maxDist = -FLT_MAX;
	for (const XMFLOAT4A &point : points)
	{
		const XMVECTOR pointVec = TO_CONST_VEC(point);
		const XMVECTOR toPoint = pointVec - origin;

		const float dot = XMVectorGetX(XMVector3Dot(toPoint, direction));

		if (dot < minDist)
			minDist = dot;

		if (dot > maxDist)
			maxDist = dot;
	}

	//_currProjInfo.nearZ = max(minDist, _defaultProjInfo.nearZ);
	_currProjInfo.farZ = min(maxDist, _defaultProjInfo.farZ);

	if (_currProjInfo.farZ - _currProjInfo.nearZ < 0.001f)
	{
		ErrMsg("Near and far planes are very close, camera could be ignored.");
		return false;
	}

	XMFLOAT4X4A projMatrix = GetProjectionMatrix();
	DirectX::BoundingFrustum::CreateFromMatrix(_frustum, *reinterpret_cast<XMMATRIX *>(&projMatrix));

	_isDirty = true;
	if (abs(currNear - _currProjInfo.nearZ) + abs(currFar - _currProjInfo.farZ) > 0.01f)
		_recalculateFrustum = true;
	return true;
}

bool CameraD3D11::UpdateBuffers(ID3D11DeviceContext *context)
{
	if (!_isDirty)
		return true;

	const XMFLOAT4X4A viewProjMatrix = GetViewProjectionMatrix();
	if (!_cameraVSBuffer.UpdateBuffer(context, &viewProjMatrix))
	{
		ErrMsg("Failed to update camera VS buffer!");
		return false;
	}

	if (_cameraGSBuffer != nullptr)
	{
		const GeometryBufferData bufferData = { viewProjMatrix, _transform.GetPosition() };
		if (!_cameraGSBuffer->UpdateBuffer(context, &bufferData))
		{
			ErrMsg("Failed to update camera GS buffer!");
			return false;
		}
	}

	if (_cameraCSBuffer != nullptr)
	{
		const XMFLOAT4A camPos = _transform.GetPosition();
		if (!_cameraCSBuffer->UpdateBuffer(context, &camPos))
		{
			ErrMsg("Failed to update camera CS buffer!");
			return false;
		}
	}

	_isDirty = false;
	return true;
}


bool CameraD3D11::BindGeometryBuffers(ID3D11DeviceContext *context) const
{
	ID3D11Buffer *const vpmBuffer = GetCameraVSBuffer();
	context->VSSetConstantBuffers(1, 1, &vpmBuffer);

	if (_cameraGSBuffer == nullptr)
	{
		ErrMsg("Failed to bind geometry buffer, camera does not have that buffer!");
		return false;
	}

	ID3D11Buffer *const camViewPosBuffer = GetCameraGSBuffer();
	context->GSSetConstantBuffers(0, 1, &camViewPosBuffer);

	return true;
}

bool CameraD3D11::BindLightingBuffers(ID3D11DeviceContext *context) const
{
	if (_cameraCSBuffer == nullptr)
	{
		ErrMsg("Failed to bind lighting buffer, camera does not have that buffer!");
		return false;
	}

	ID3D11Buffer *const camPosBuffer = GetCameraCSBuffer();
	context->CSSetConstantBuffers(1, 1, &camPosBuffer);

	return true;
}


void CameraD3D11::StoreFrustum(DirectX::BoundingFrustum &frustum)
{
	if (_recalculateFrustum)
	{
		_frustum.Transform(_transformedFrustum, _transform.GetWorldMatrix());
		_recalculateFrustum = false;
	}

	frustum = _transformedFrustum;
}


void CameraD3D11::QueueEmitter(const RenderInstance &emitter)
{
	_particleEmitters.push_back(emitter);
}

void CameraD3D11::QueueRenderInstance(const ResourceGroup &resources, const RenderInstance &instance)
{
	_renderInstances.insert({ resources, instance });
}

void CameraD3D11::ResetRenderQueue()
{
	_lastCullCount = _renderInstances.size();
	_renderInstances.clear();
	_particleEmitters.clear();
}


const std::vector<RenderInstance> &CameraD3D11::GetEmitterQueue() const
{
	return _particleEmitters;
}

const std::multimap<ResourceGroup, RenderInstance> &CameraD3D11::GetRenderQueue() const
{
	return _renderInstances;
}

UINT CameraD3D11::GetCullCount() const
{
	return _lastCullCount;
}


const Transform &CameraD3D11::GetTransform() const 
{
	return _transform;
}

ID3D11Buffer *CameraD3D11::GetCameraVSBuffer() const
{
	return _cameraVSBuffer.GetBuffer();
}

ID3D11Buffer *CameraD3D11::GetCameraGSBuffer() const
{
	if (_cameraGSBuffer == nullptr)
		return nullptr;

	return _cameraGSBuffer->GetBuffer();
}

ID3D11Buffer *CameraD3D11::GetCameraCSBuffer() const
{
	if (_cameraCSBuffer == nullptr)
		return nullptr;

	return _cameraCSBuffer->GetBuffer();
}
