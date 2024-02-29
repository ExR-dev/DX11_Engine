#include "CameraD3D11.h"

#include "ErrMsg.h"


void CameraD3D11::Move(const float amount, const XMFLOAT4A &direction)
{
	_transform.Move({
		direction.x * amount,
		direction.y * amount,
		direction.z * amount,
		0.0f
	});
	_isDirty = true;
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
}


CameraD3D11::CameraD3D11(ID3D11Device *device, const ProjectionInfo &projectionInfo, const XMFLOAT4A &initialPosition, const bool hasCSBuffer)
{
	if (!Initialize(device, projectionInfo, initialPosition, hasCSBuffer))
		ErrMsg("Failed to initialize camera buffer in camera constructor!");
}

CameraD3D11::~CameraD3D11()
{
	delete _cameraCSBuffer;
}


bool CameraD3D11::Initialize(ID3D11Device *device, const ProjectionInfo &projectionInfo, const XMFLOAT4A &initialPosition, const bool hasCSBuffer)
{
	_projInfo = projectionInfo;
	_transform.SetPosition(initialPosition);

	const XMFLOAT4X4 viewProjMatrix = GetViewProjectionMatrix();
	if (!_cameraVSBuffer.Initialize(device, sizeof(XMFLOAT4X4), &viewProjMatrix))
	{
		ErrMsg("Failed to initialize camera VS buffer!");
		return false;
	}

	if (hasCSBuffer)
	{
		_cameraCSBuffer = new ConstantBufferD3D11();
		if (!_cameraCSBuffer->Initialize(device, sizeof(XMFLOAT4A), &initialPosition))
		{
			ErrMsg("Failed to initialize camera CS buffer!");
			return false;
		}
	}

	return true;
}


void CameraD3D11::MoveForward(const float amount)
{
	MoveLocal(amount, { 0, 0, 1, 0 });
	_isDirty = true;
}

void CameraD3D11::MoveRight(const float amount)
{
	MoveLocal(amount, { 1, 0, 0, 0 });
	_isDirty = true;
}

void CameraD3D11::MoveUp(const float amount)
{
	MoveLocal(amount, { 0, 1, 0, 0 });
	_isDirty = true;
}


void CameraD3D11::RotateRoll(const float amount)
{
	_transform.RotateLocal({ 0, 0, amount, 0 });
	_isDirty = true;
}

void CameraD3D11::RotatePitch(const float amount)
{
	_transform.RotateLocal({ amount, 0, 0, 0 });
	_isDirty = true;
}

void CameraD3D11::RotateYaw(const float amount)
{
	_transform.RotateLocal({ 0, amount, 0, 0 });
	_isDirty = true;
}


void CameraD3D11::LookX(const float amount)
{
	_transform.Rotate({ 0, amount, 0, 0 });
	_isDirty = true;
}

void CameraD3D11::LookY(const float amount)
{
	_transform.RotateLocal({ -amount, 0, 0, 0 });
	_isDirty = true;
}


const XMFLOAT4A &CameraD3D11::GetRight() const		{ return _transform.GetRight();		}
const XMFLOAT4A &CameraD3D11::GetUp() const			{ return _transform.GetUp();		}
const XMFLOAT4A &CameraD3D11::GetForward() const	{ return _transform.GetForward();	}
const XMFLOAT4A &CameraD3D11::GetPosition() const	{ return _transform.GetPosition();	}

XMFLOAT4X4 CameraD3D11::GetViewProjectionMatrix() const
{
	XMFLOAT4A
		cPos = _transform.GetPosition(),
		cFwd = _transform.GetForward(),
		cUp = _transform.GetUp();

	XMFLOAT4X4 vpMatrix;

	XMStoreFloat4x4(
		&vpMatrix,
		XMMatrixTranspose(
			XMMatrixLookAtLH(
				*reinterpret_cast<XMVECTOR *>(&cPos),
				*reinterpret_cast<XMVECTOR *>(&cPos) + *reinterpret_cast<XMVECTOR *>(&cFwd),
				*reinterpret_cast<XMVECTOR *>(&cUp)
			) *
			XMMatrixPerspectiveFovLH(
				_projInfo.fovAngleY * (XM_PI / 180.0f),
				_projInfo.aspectRatio,
				_projInfo.nearZ,
				_projInfo.farZ
			)
		)
	);

	return vpMatrix;
}


bool CameraD3D11::UpdateBuffers(ID3D11DeviceContext *context)
{
	if (!_isDirty)
		return true;

	const XMFLOAT4X4 viewProjMatrix = GetViewProjectionMatrix();
	if (!_cameraVSBuffer.UpdateBuffer(context, &viewProjMatrix))
	{
		ErrMsg("Failed to update camera VS buffer!");
		return false;
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

	return true;
}

bool CameraD3D11::BindLightingBuffers(ID3D11DeviceContext *context) const
{
	if (_cameraCSBuffer == nullptr)
	{
		ErrMsg("Failed to bind lighting buffers, camera does not have that buffer!");
		return false;
	}

	ID3D11Buffer *const camPosBuffer = GetCameraCSBuffer();
	context->CSSetConstantBuffers(1, 1, &camPosBuffer);

	return true;
}


ID3D11Buffer *CameraD3D11::GetCameraVSBuffer() const
{
	return _cameraVSBuffer.GetBuffer();
}

ID3D11Buffer *CameraD3D11::GetCameraCSBuffer() const
{
	if (_cameraCSBuffer == nullptr)
		return nullptr;

	return _cameraCSBuffer->GetBuffer();
}
