#include "CameraD3D11.h"

#include <algorithm>

#include "ErrMsg.h"


using namespace DirectX;


#define TO_VEC(x)		( *reinterpret_cast<XMVECTOR *>(&(x))		)
#define TO_CONST_VEC(x) ( *reinterpret_cast<const XMVECTOR *>(&(x))	)


CameraD3D11::CameraD3D11(ID3D11Device *device, const ProjectionInfo &projectionInfo, const XMFLOAT4A &initialPosition, const bool hasCSBuffer, const bool isOrtho)
{
	if (!Initialize(device, projectionInfo, initialPosition, hasCSBuffer, isOrtho))
		ErrMsg("Failed to initialize camera buffer in camera constructor!");
}

CameraD3D11::~CameraD3D11()
{
	delete _viewProjPosBuffer;
	delete _posBuffer;
}


bool CameraD3D11::Initialize(ID3D11Device *device, const ProjectionInfo &projectionInfo, const XMFLOAT4A &initialPosition, const bool hasCSBuffer, const bool isOrtho)
{
	_ortho = isOrtho;
	_currProjInfo = _defaultProjInfo = projectionInfo;
	_transform.SetPosition(initialPosition);

	const XMFLOAT4X4A viewProjMatrix = GetViewProjectionMatrix();
	if (!_viewProjBuffer.Initialize(device, sizeof(XMFLOAT4X4A), &viewProjMatrix))
	{
		ErrMsg("Failed to initialize camera VS buffer!");
		return false;
	}

	if (hasCSBuffer)
	{
		const GeometryBufferData bufferData = { GetViewMatrix(), _transform.GetPosition() };
		_viewProjPosBuffer = new ConstantBufferD3D11();
		if (!_viewProjPosBuffer->Initialize(device, sizeof(GeometryBufferData), &bufferData))
		{
			ErrMsg("Failed to initialize camera GS buffer!");
			return false;
		}

		_posBuffer = new ConstantBufferD3D11();
		if (!_posBuffer->Initialize(device, sizeof(XMFLOAT4A), &initialPosition))
		{
			ErrMsg("Failed to initialize camera CS buffer!");
			return false;
		}
	}

	if (_ortho)
	{
		const float
			nearZ = _currProjInfo.nearZ,
			farZ = _currProjInfo.farZ,
			width = 0.5f * _currProjInfo.fovAngleY * _currProjInfo.aspectRatio,
			height = 0.5f * _currProjInfo.fovAngleY;

		const XMFLOAT3 corners[8] = {
			XMFLOAT3(-width, -height, nearZ),
			XMFLOAT3(width, -height, nearZ),
			XMFLOAT3(-width,  height, nearZ),
			XMFLOAT3(width,  height, nearZ),
			XMFLOAT3(-width, -height, farZ),
			XMFLOAT3(width, -height, farZ),
			XMFLOAT3(-width,  height, farZ),
			XMFLOAT3(width,  height, farZ)
		};

		BoundingOrientedBox::CreateFromPoints(_bounds.ortho, 8, corners, sizeof(XMFLOAT3));
	}
	else
	{
		const XMFLOAT4X4A projMatrix = GetProjectionMatrix();
		BoundingFrustum::CreateFromMatrix(_bounds.perspective, *reinterpret_cast<const XMMATRIX *>(&projMatrix));
	}

	return true;
}


void CameraD3D11::Move(const float amount, const XMFLOAT4A &direction)
{
	_transform.Move({
		direction.x * amount,
		direction.y * amount,
		direction.z * amount,
		0.0f
	});
	_isDirty = true;
	_recalculateBounds = true;
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
	_recalculateBounds = true;
}


void CameraD3D11::MoveForward(const float amount)
{
	MoveLocal(amount, { 0.0f, 0.0f, 1.0f, 0.0f });
	_isDirty = true;
	_recalculateBounds = true;
}

void CameraD3D11::MoveRight(const float amount)
{
	MoveLocal(amount, { 1.0f, 0.0f, 0.0f, 0.0f });
	_isDirty = true;
	_recalculateBounds = true;
}

void CameraD3D11::MoveUp(const float amount)
{
	MoveLocal(amount, { 0.0f, 1.0f, 0.0f, 0.0f });
	_isDirty = true;
	_recalculateBounds = true;
}


void CameraD3D11::RotateRoll(const float amount)
{
	_transform.RotateLocal({ 0.0f, 0.0f, amount, 0.0f });
	_isDirty = true;
	_recalculateBounds = true;
}

void CameraD3D11::RotatePitch(const float amount)
{
	_transform.RotateLocal({ amount, 0.0f, 0.0f, 0.0f });
	_isDirty = true;
	_recalculateBounds = true;
}

void CameraD3D11::RotateYaw(const float amount)
{
	_transform.RotateLocal({ 0.0f, amount, 0.0f, 0.0f });
	_isDirty = true;
	_recalculateBounds = true;
}


void CameraD3D11::LookX(const float amount)
{
	_transform.Rotate({ 0.0f, amount * (XMVectorGetX(XMVector3Dot(TO_CONST_VEC(_transform.GetUp()), {0, 1, 0, 0})) > 0.0f ? 1.0f : -1.0f), 0.0f, 0.0f });
	_isDirty = true;
	_recalculateBounds = true;
}

void CameraD3D11::LookY(const float amount)
{
	_transform.RotateLocal({ amount, 0.0f, 0.0f, 0.0f });
	_isDirty = true;
	_recalculateBounds = true;
}

void CameraD3D11::SetFOV(const float amount)
{
	_currProjInfo.fovAngleY = amount;

	if (_ortho)
	{
		const float
			nearZ = _currProjInfo.nearZ,
			farZ = _currProjInfo.farZ,
			width = 0.5f * _currProjInfo.fovAngleY * _currProjInfo.aspectRatio,
			height = 0.5f * _currProjInfo.fovAngleY;

		const XMFLOAT3 corners[8] = {
			XMFLOAT3(-width, -height, nearZ),
			XMFLOAT3(width, -height, nearZ),
			XMFLOAT3(-width,  height, nearZ),
			XMFLOAT3(width,  height, nearZ),
			XMFLOAT3(-width, -height, farZ),
			XMFLOAT3(width, -height, farZ),
			XMFLOAT3(-width,  height, farZ),
			XMFLOAT3(width,  height, farZ)
		};

		BoundingOrientedBox::CreateFromPoints(_bounds.ortho, 8, corners, sizeof(XMFLOAT3));
	}
	else
	{
		const XMFLOAT4X4A projMatrix = GetProjectionMatrix();
		BoundingFrustum::CreateFromMatrix(_bounds.perspective, *reinterpret_cast<const XMMATRIX *>(&projMatrix));
	}

	_isDirty = true;
	_recalculateBounds = true;
}

void CameraD3D11::SetOrtho(bool state)
{
	_ortho = state;

	if (_ortho)
	{
		const float
			nearZ = _currProjInfo.nearZ,
			farZ = _currProjInfo.farZ,
			width = 0.5f * _currProjInfo.fovAngleY * _currProjInfo.aspectRatio,
			height = 0.5f * _currProjInfo.fovAngleY;

		const XMFLOAT3 corners[8] = {
			XMFLOAT3(-width, -height, nearZ),
			XMFLOAT3( width, -height, nearZ),
			XMFLOAT3(-width,  height, nearZ),
			XMFLOAT3( width,  height, nearZ),
			XMFLOAT3(-width, -height, farZ),
			XMFLOAT3( width, -height, farZ),
			XMFLOAT3(-width,  height, farZ),
			XMFLOAT3( width,  height, farZ)
		};

		BoundingOrientedBox::CreateFromPoints(_bounds.ortho, 8, corners, sizeof(XMFLOAT3));
	}
	else
	{
		const XMFLOAT4X4A projMatrix = GetProjectionMatrix();
		BoundingFrustum::CreateFromMatrix(_bounds.perspective, *reinterpret_cast<const XMMATRIX*>(&projMatrix));
	}

	_isDirty = true;
	_recalculateBounds = true;
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
	XMMATRIX projectionMatrix;

	if (_ortho)
	{
		projectionMatrix = XMMatrixOrthographicLH(
			_currProjInfo.fovAngleY * _currProjInfo.aspectRatio,
			_currProjInfo.fovAngleY,
			_currProjInfo.nearZ,
			_currProjInfo.farZ
		);
	}
	else
	{
		projectionMatrix = XMMatrixPerspectiveFovLH(
			_currProjInfo.fovAngleY,
			_currProjInfo.aspectRatio,
			_currProjInfo.nearZ,
			_currProjInfo.farZ
		);
	}

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
			) * (_ortho ? 
			XMMatrixOrthographicLH(
				_currProjInfo.fovAngleY * _currProjInfo.aspectRatio,
				_currProjInfo.fovAngleY,
				_currProjInfo.farZ,
				_currProjInfo.nearZ
			) :
			XMMatrixPerspectiveFovLH(
				_currProjInfo.fovAngleY,
				_currProjInfo.aspectRatio,
				_currProjInfo.farZ,
				_currProjInfo.nearZ
			))
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

	if (_ortho)
	{
		const float
			nearZ = _currProjInfo.nearZ,
			farZ = _currProjInfo.farZ,
			width = 0.5f * _currProjInfo.fovAngleY * _currProjInfo.aspectRatio,
			height = 0.5f * _currProjInfo.fovAngleY;

		const XMFLOAT3 corners[8] = {
			XMFLOAT3( -width, -height, nearZ ),
			XMFLOAT3(  width, -height, nearZ ),
			XMFLOAT3( -width,  height, nearZ ),
			XMFLOAT3(  width,  height, nearZ ),
			XMFLOAT3( -width, -height, farZ  ),
			XMFLOAT3(  width, -height, farZ  ),
			XMFLOAT3( -width,  height, farZ  ),
			XMFLOAT3(  width,  height, farZ  )
		};

		BoundingOrientedBox::CreateFromPoints(_bounds.ortho, 8, corners, sizeof(XMFLOAT3));
	}
	else
	{
		const XMFLOAT4X4A projMatrix = GetProjectionMatrix();
		BoundingFrustum::CreateFromMatrix(_bounds.perspective, *reinterpret_cast<const XMMATRIX *>(&projMatrix));
	}

	if (abs(currNear - _currProjInfo.nearZ) + abs(currFar - _currProjInfo.farZ) > 0.01f)
	{
		_isDirty = true;
		_recalculateBounds = true;
	}
	return true;
}

bool CameraD3D11::UpdateBuffers(ID3D11DeviceContext *context)
{
	if (!_isDirty)
		return true;

	const XMFLOAT4X4A viewProjMatrix = GetViewProjectionMatrix();
	if (!_viewProjBuffer.UpdateBuffer(context, &viewProjMatrix))
	{
		ErrMsg("Failed to update camera view projection buffer!");
		return false;
	}

	if (_viewProjPosBuffer != nullptr)
	{
		const GeometryBufferData bufferData = { viewProjMatrix, _transform.GetPosition() };
		if (!_viewProjPosBuffer->UpdateBuffer(context, &bufferData))
		{
			ErrMsg("Failed to update camera view projection positon buffer!");
			return false;
		}
	}

	if (_posBuffer != nullptr)
		if (!_posBuffer->UpdateBuffer(context, &_transform.GetPosition()))
		{
			ErrMsg("Failed to update camera position buffer!");
			return false;
		}

	_isDirty = false;
	return true;
}


bool CameraD3D11::BindShadowCasterBuffers(ID3D11DeviceContext *context) const
{
	ID3D11Buffer *const vpmBuffer = GetCameraVSBuffer();
	context->VSSetConstantBuffers(1, 1, &vpmBuffer);

	return true;
}

bool CameraD3D11::BindGeometryBuffers(ID3D11DeviceContext *context) const
{
	ID3D11Buffer *const posBuffer = (_posBuffer == nullptr) ? nullptr : _posBuffer->GetBuffer();
	context->HSSetConstantBuffers(1, 1, &posBuffer);

	ID3D11Buffer *const vpmBuffer = GetCameraVSBuffer();
	context->DSSetConstantBuffers(0, 1, &vpmBuffer);

	return true;
}

bool CameraD3D11::BindLightingBuffers(ID3D11DeviceContext *context) const
{
	ID3D11Buffer *const camPosBuffer = GetCameraCSBuffer();
	if (camPosBuffer == nullptr)
	{
		ErrMsg("Failed to bind lighting buffer, camera does not have that buffer!");
		return false;
	}
	context->CSSetConstantBuffers(1, 1, &camPosBuffer);

	return true;
}

bool CameraD3D11::BindTransparentBuffers(ID3D11DeviceContext *context) const
{
	ID3D11Buffer *const vpmBuffer = GetCameraVSBuffer();
	context->DSSetConstantBuffers(0, 1, &vpmBuffer);

	ID3D11Buffer *const camViewPosBuffer = GetCameraGSBuffer();
	if (camViewPosBuffer == nullptr)
	{
		ErrMsg("Failed to bind geometry buffer, camera does not have that buffer!");
		return false;
	}
	context->GSSetConstantBuffers(0, 1, &camViewPosBuffer);

	return true;
}

bool CameraD3D11::BindViewBuffers(ID3D11DeviceContext *context) const
{
	ID3D11Buffer *const vpmBuffer = GetCameraVSBuffer();
	context->DSSetConstantBuffers(0, 1, &vpmBuffer);

	return true;
}

bool CameraD3D11::BindMainBuffers(ID3D11DeviceContext *context) const
{
	ID3D11Buffer *const posBuffer = (_posBuffer == nullptr) ? nullptr : _posBuffer->GetBuffer();
	context->HSSetConstantBuffers(1, 1, &posBuffer);

	return true;
}


bool CameraD3D11::StoreBounds(BoundingFrustum &bounds)
{
	if (_ortho)
		return false;

	if (_recalculateBounds)
	{
		_bounds.perspective.Transform(_transformedBounds.perspective, _transform.GetWorldMatrix());
		_recalculateBounds = false;
	}

	bounds = _transformedBounds.perspective;
	return true;
}

bool CameraD3D11::StoreBounds(BoundingOrientedBox &bounds)
{
	if (!_ortho)
		return false;

	if (_recalculateBounds)
	{
		_bounds.ortho.Transform(_transformedBounds.ortho, _transform.GetWorldMatrix());
		_recalculateBounds = false;
	}

	bounds = _transformedBounds.ortho;
	return true;
}


void CameraD3D11::QueueGeometry(const ResourceGroup &resources, const RenderInstance &instance)
{
	_geometryRenderQueue.insert({ resources, instance });
}

void CameraD3D11::QueueTransparent(const ResourceGroup &resources, const RenderInstance &instance)
{
	_transparentRenderQueue.insert({ resources, instance });
}

void CameraD3D11::QueueEmitter(const ResourceGroup &resources, const RenderInstance &instance)
{
	_particleRenderQueue.insert({ resources, instance });
}

void CameraD3D11::ResetRenderQueue()
{
	_lastCullCount = static_cast<UINT>(
		_geometryRenderQueue.size() + 
		_transparentRenderQueue.size() + 
		_particleRenderQueue.size()
	);

	_geometryRenderQueue.clear();
	_transparentRenderQueue.clear();
	_particleRenderQueue.clear();
}


UINT CameraD3D11::GetCullCount() const
{
	return _lastCullCount;
}

const std::multimap<ResourceGroup, RenderInstance> &CameraD3D11::GetGeometryQueue() const
{
	return _geometryRenderQueue;
}

const std::multimap<ResourceGroup, RenderInstance> &CameraD3D11::GetTransparentQueue() const
{
	return _transparentRenderQueue;
}

const std::multimap<ResourceGroup, RenderInstance> &CameraD3D11::GetParticleQueue() const
{
	return _particleRenderQueue;
}


bool CameraD3D11::GetOrtho() const
{
	return _ortho;
}

const Transform &CameraD3D11::GetTransform() const 
{
	return _transform;
}

ID3D11Buffer *CameraD3D11::GetCameraVSBuffer() const
{
	return _viewProjBuffer.GetBuffer();
}

ID3D11Buffer *CameraD3D11::GetCameraGSBuffer() const
{
	if (_viewProjPosBuffer == nullptr)
		return nullptr;

	return _viewProjPosBuffer->GetBuffer();
}

ID3D11Buffer *CameraD3D11::GetCameraCSBuffer() const
{
	if (_posBuffer == nullptr)
		return nullptr;

	return _posBuffer->GetBuffer();
}
