#include "CameraD3D11.h"
#include <algorithm>
#include "ErrMsg.h"

using namespace DirectX;
using namespace SimpleMath;

#define TO_VEC(x)		( *reinterpret_cast<XMVECTOR *>(&(x))		)
#define TO_CONST_VEC(x) ( *reinterpret_cast<const XMVECTOR *>(&(x))	)


CameraD3D11::CameraD3D11(ID3D11Device *device, const ProjectionInfo &projectionInfo, const Vector3 &initialPosition, const bool hasCSBuffer, const bool isOrtho)
{
	if (!Initialize(device, projectionInfo, initialPosition, hasCSBuffer, isOrtho))
		ErrMsg("Failed to initialize camera buffer in camera constructor!");
}

CameraD3D11::~CameraD3D11()
{
	delete _viewProjPosBuffer;
	delete _posBuffer;
}


bool CameraD3D11::Initialize(ID3D11Device *device, const ProjectionInfo &projectionInfo, const Vector3 &initialPosition, const bool hasCSBuffer, const bool isOrtho)
{
	_ortho = isOrtho;
	_currProjInfo = _defaultProjInfo = projectionInfo;
	_transform.SetPosition(initialPosition);

	const Matrix viewProjMatrix = GetViewProjectionMatrix();
	if (!_viewProjBuffer.Initialize(device, sizeof(Matrix), &viewProjMatrix))
	{
		ErrMsg("Failed to initialize camera VS buffer!");
		return false;
	}

	if (hasCSBuffer)
	{
		Vector4 paddedPos = { initialPosition.x, initialPosition.y, initialPosition.z, 0 };

		const GeometryBufferData bufferData = { GetViewMatrix(), paddedPos };
		_viewProjPosBuffer = new ConstantBufferD3D11();
		if (!_viewProjPosBuffer->Initialize(device, sizeof(GeometryBufferData), &bufferData))
		{
			ErrMsg("Failed to initialize camera GS buffer!");
			return false;
		}

		_posBuffer = new ConstantBufferD3D11();
		if (!_posBuffer->Initialize(device, sizeof(Vector4), &paddedPos))
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

		const Vector3 corners[8] = {
			Vector3(-width, -height, nearZ),
			Vector3(width, -height, nearZ),
			Vector3(-width,  height, nearZ),
			Vector3(width,  height, nearZ),
			Vector3(-width, -height, farZ),
			Vector3(width, -height, farZ),
			Vector3(-width,  height, farZ),
			Vector3(width,  height, farZ)
		};

		BoundingOrientedBox::CreateFromPoints(_bounds.ortho, 8, corners, sizeof(Vector3));
	}
	else
	{
		const Matrix projMatrix = GetProjectionMatrix();
		BoundingFrustum::CreateFromMatrix(_bounds.perspective, projMatrix);
	}

	return true;
}


void CameraD3D11::Move(const float amount, const Vector3 &direction)
{
	_transform.Move({
		direction.x * amount,
		direction.y * amount,
		direction.z * amount
	});
	_isDirty = true;
	_recalculateBounds = true;
}

void CameraD3D11::MoveLocal(const float amount, const Vector3 &direction)
{
	_transform.Move({
		direction.x * amount,
		direction.y * amount,
		direction.z * amount
	}, Local);
	_isDirty = true;
	_recalculateBounds = true;
}


void CameraD3D11::MoveForward(const float amount)
{
	MoveLocal(amount, { 0.0f, 0.0f, 1.0f });
	_isDirty = true;
	_recalculateBounds = true;
}

void CameraD3D11::MoveRight(const float amount)
{
	MoveLocal(amount, { 1.0f, 0.0f, 0.0f });
	_isDirty = true;
	_recalculateBounds = true;
}

void CameraD3D11::MoveUp(const float amount)
{
	MoveLocal(amount, { 0.0f, 1.0f, 0.0f });
	_isDirty = true;
	_recalculateBounds = true;
}


void CameraD3D11::RotateRoll(const float amount)
{
	_transform.Rotate({ 0.0f, 0.0f, amount }, Local);
	_isDirty = true;
	_recalculateBounds = true;
}

void CameraD3D11::RotatePitch(const float amount)
{
	_transform.Rotate({ amount, 0.0f, 0.0f }, Local);
	_isDirty = true;
	_recalculateBounds = true;
}

void CameraD3D11::RotateYaw(const float amount)
{
	_transform.Rotate({ 0.0f, amount, 0.0f }, Local);
	_isDirty = true;
	_recalculateBounds = true;
}


void CameraD3D11::LookX(const float amount)
{
	Vector3 upVec = _transform.Up();
	XMFLOAT4A up = { upVec.x, upVec.y, upVec.z, 0 };

	_transform.Rotate({ 0.0f, amount * (XMVectorGetX(XMVector3Dot(TO_CONST_VEC(up), {0, 1, 0, 0})) > 0.0f ? 1.0f : -1.0f), 0.0f });
	_isDirty = true;
	_recalculateBounds = true;
}

void CameraD3D11::LookY(const float amount)
{
	_transform.Rotate({ amount, 0.0f, 0.0f }, Local);
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

		const Vector3 corners[8] = {
			Vector3(-width, -height, nearZ),
			Vector3(width, -height, nearZ),
			Vector3(-width,  height, nearZ),
			Vector3(width,  height, nearZ),
			Vector3(-width, -height, farZ),
			Vector3(width, -height, farZ),
			Vector3(-width,  height, farZ),
			Vector3(width,  height, farZ)
		};

		BoundingOrientedBox::CreateFromPoints(_bounds.ortho, 8, corners, sizeof(Vector3));
	}
	else
	{
		const Matrix projMatrix = GetProjectionMatrix();
		BoundingFrustum::CreateFromMatrix(_bounds.perspective, *reinterpret_cast<const Matrix *>(&projMatrix));
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

		const Vector3 corners[8] = {
			Vector3(-width, -height, nearZ),
			Vector3( width, -height, nearZ),
			Vector3(-width,  height, nearZ),
			Vector3( width,  height, nearZ),
			Vector3(-width, -height, farZ),
			Vector3( width, -height, farZ),
			Vector3(-width,  height, farZ),
			Vector3( width,  height, farZ)
		};

		BoundingOrientedBox::CreateFromPoints(_bounds.ortho, 8, corners, sizeof(Vector3));
	}
	else
	{
		const Matrix projMatrix = GetProjectionMatrix();
		BoundingFrustum::CreateFromMatrix(_bounds.perspective, *reinterpret_cast<const Matrix *>(&projMatrix));
	}

	_isDirty = true;
	_recalculateBounds = true;
}


Vector3 CameraD3D11::GetPosition() 	{ return _transform.GetPosition();	}
Vector3 CameraD3D11::GetRight() 	{ return _transform.Right();		}
Vector3 CameraD3D11::GetUp() 		{ return _transform.Up();			}
Vector3 CameraD3D11::GetForward() 	{ return _transform.Forward();		}


Matrix CameraD3D11::GetViewMatrix()
{
	const Vector3
		cPos = _transform.GetPosition(),
		cFwd = _transform.Forward(),
		cUp = _transform.Up();

	return Matrix::CreateLookAt(cPos, cPos + cFwd, cUp);
}

Matrix CameraD3D11::GetProjectionMatrix()
{
	Matrix projectionMatrix;

	if (_ortho)
	{
		XMStoreFloat4x4(&projectionMatrix, XMMatrixOrthographicLH(
			_currProjInfo.fovAngleY * _currProjInfo.aspectRatio,
			_currProjInfo.fovAngleY,
			_currProjInfo.nearZ,
			_currProjInfo.farZ
		));

		/*projectionMatrix = Matrix::CreateOrthographic( // TODO: RH instead of LH, is this a problem?
			_currProjInfo.fovAngleY * _currProjInfo.aspectRatio,
			_currProjInfo.fovAngleY,
			_currProjInfo.nearZ,
			_currProjInfo.farZ
		);*/
	}
	else
	{
		XMStoreFloat4x4(&projectionMatrix, XMMatrixPerspectiveFovLH(
			_currProjInfo.fovAngleY,
			_currProjInfo.aspectRatio,
			_currProjInfo.nearZ,
			_currProjInfo.farZ
		));

		/*projectionMatrix = Matrix::CreatePerspectiveFieldOfView( // TODO: RH instead of LH, is this a problem?
			_currProjInfo.fovAngleY,
			_currProjInfo.aspectRatio,
			_currProjInfo.nearZ,
			_currProjInfo.farZ
		);*/
	}

	return projectionMatrix;
}

Matrix CameraD3D11::GetViewProjectionMatrix()
{
	return XMMatrixTranspose(GetViewMatrix() * GetProjectionMatrix());
}

const ProjectionInfo &CameraD3D11::GetCurrProjectionInfo() const
{
	return _currProjInfo;
}


bool CameraD3D11::ScaleToContents(const std::vector<Vector3> &nearBounds, const std::vector<Vector3> &innerBounds)
{
	if (!_ortho)
		return false;

	const Vector3
		forward = _transform.Forward(),
		right = _transform.Right(),
		up = _transform.Up();

	Vector3 mid = { 0.0f, 0.0f, 0.0f};
	for (const Vector3 &point : innerBounds)
		mid += point;
	mid /= static_cast<float>(innerBounds.size());

	float
		nearDist = FLT_MAX, farDist = -FLT_MAX,
		leftDist = FLT_MAX, rightDist = -FLT_MAX,
		downDist = FLT_MAX, upDist = -FLT_MAX;

	float
		sceneFarDist = -FLT_MAX,
		sceneLeftDist = FLT_MAX, sceneRightDist = -FLT_MAX,
		sceneDownDist = FLT_MAX, sceneUpDist = -FLT_MAX;

	for (const Vector3 &point : nearBounds)
	{
		const Vector3 toPoint = point - mid;

		const float
			xDot = toPoint.Dot(right),
			yDot = toPoint.Dot(up),
			zDot = toPoint.Dot(forward);

		if (xDot < sceneLeftDist)	sceneLeftDist = xDot;
		if (xDot > sceneRightDist)	sceneRightDist = xDot;

		if (yDot < sceneDownDist)	sceneDownDist = yDot;
		if (yDot > sceneUpDist)		sceneUpDist = yDot;

		if (zDot < nearDist)		nearDist = zDot;
		if (zDot > sceneFarDist)	sceneFarDist = zDot;
	}

	for (const Vector3 &point : innerBounds)
	{
		const Vector3 toPoint = point - mid;

		const float
			xDot = toPoint.Dot(right),
			yDot = toPoint.Dot(up),
			zDot = toPoint.Dot(forward);

		if (xDot < leftDist)	leftDist = xDot;
		if (xDot > rightDist)	rightDist = xDot;

		if (yDot < downDist)	downDist = yDot;
		if (yDot > upDist)		upDist = yDot;

		if (zDot > farDist)		farDist = zDot;
	}

	if (sceneLeftDist > leftDist)	leftDist = sceneLeftDist;
	if (sceneRightDist < rightDist)	rightDist = sceneRightDist;
	if (sceneDownDist > downDist)	downDist = sceneDownDist;
	if (sceneUpDist < upDist)		upDist = sceneUpDist;

	if (farDist - nearDist < 0.001f)
	{
		ErrMsg("Near and far planes are very close, camera can likely be disabled.");
		return false;
	}

	Vector3 newPos = mid + 
		(forward * (nearDist - 1.0f)) + 
		(right * ((rightDist + leftDist) * 0.5f)) +
		(up * ((upDist + downDist) * 0.5f));

	_transform.SetPosition(newPos, Local);

	const float
		nearZ = 1.0f,
		farZ = (farDist - nearDist) + 1.0f,
		width = (rightDist - leftDist) * 0.5f,
		height = (upDist - downDist) * 0.5f;

	const Vector3 corners[8] = {
		Vector3(-width, -height, nearZ),
		Vector3( width, -height, nearZ),
		Vector3(-width,  height, nearZ),
		Vector3( width,  height, nearZ),
		Vector3(-width, -height, farZ),
		Vector3( width, -height, farZ),
		Vector3(-width,  height, farZ),
		Vector3( width,  height, farZ)
	};

	BoundingOrientedBox::CreateFromPoints(_bounds.ortho, 8, corners, sizeof(Vector3));

	_currProjInfo.nearZ = nearZ;
	_currProjInfo.farZ = farZ;
	_currProjInfo.fovAngleY = height * 2.0f;
	_currProjInfo.aspectRatio = width / height;

	_isDirty = true;
	return true;
}

bool CameraD3D11::FitPlanesToPoints(const std::vector<Vector3> &points)
{
	const float
		currNear = _currProjInfo.nearZ,
		currFar = _currProjInfo.farZ;

	const Vector3 origin = _transform.GetPosition();
	const Vector3 direction = _transform.Forward();

	float minDist = FLT_MAX, maxDist = -FLT_MAX;
	for (const Vector3 &point : points)
	{
		const Vector3 toPoint = point - origin;

		const float dot = toPoint.Dot(direction);

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

		const Vector3 corners[8] = {
			Vector3( -width, -height, nearZ ),
			Vector3(  width, -height, nearZ ),
			Vector3( -width,  height, nearZ ),
			Vector3(  width,  height, nearZ ),
			Vector3( -width, -height, farZ  ),
			Vector3(  width, -height, farZ  ),
			Vector3( -width,  height, farZ  ),
			Vector3(  width,  height, farZ  )
		};

		BoundingOrientedBox::CreateFromPoints(_bounds.ortho, 8, corners, sizeof(Vector3));
	}
	else
	{
		const Matrix projMatrix = GetProjectionMatrix();
		BoundingFrustum::CreateFromMatrix(_bounds.perspective, projMatrix);
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

	const Matrix viewProjMatrix = GetViewProjectionMatrix();
	if (!_viewProjBuffer.UpdateBuffer(context, &viewProjMatrix))
	{
		ErrMsg("Failed to update camera view projection buffer!");
		return false;
	}

	if (_viewProjPosBuffer != nullptr)
	{
		Vector3 pos = _transform.GetPosition();
		XMFLOAT4A paddedPos = { pos.x, pos.y, pos.z, 0 };

		const GeometryBufferData bufferData = { viewProjMatrix, paddedPos };
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

Transform &CameraD3D11::GetTransform()
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
