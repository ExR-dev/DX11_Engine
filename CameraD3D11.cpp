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


CameraD3D11::CameraD3D11(ID3D11Device *device, const ProjectionInfo &projectionInfo, const XMFLOAT4A &initialPosition)
{
	if (!Initialize(device, projectionInfo, initialPosition))
		ErrMsg("Failed to initialize camera buffer in camera constructor!");
}

bool CameraD3D11::Initialize(ID3D11Device *device, const ProjectionInfo &projectionInfo, const XMFLOAT4A &initialPosition)
{
	_projInfo = projectionInfo;
	_transform.SetPosition(initialPosition);

	const XMFLOAT4X4 viewProjMatrix = GetViewProjectionMatrix();
	if (!_cameraBuffer.Initialize(device, sizeof(XMFLOAT4X4), &viewProjMatrix))
	{
		ErrMsg("Failed to initialize camera buffer!");
		return false;
	}

	std::memcpy(&_lightingBufferData.camPos[0], &initialPosition.x, sizeof(float) * 3);
	if (!_lightingBuffer.Initialize(device, sizeof(LightingBufferData), &_lightingBufferData))
	{
		ErrMsg("Failed to initialize lighting buffer!");
		return false;
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


void CameraD3D11::RotateForward(const float amount)
{
	_transform.RotateLocal({ 0, 0, amount, 0 });
	_isDirty = true;
}

void CameraD3D11::RotateRight(const float amount)
{
	_transform.RotateLocal({ amount, 0, 0, 0 });
	_isDirty = true;
}

void CameraD3D11::RotateUp(const float amount)
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

const XMFLOAT4X4 &CameraD3D11::GetViewProjectionMatrix() const
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
	if (!_cameraBuffer.UpdateBuffer(context, &viewProjMatrix))
	{
		ErrMsg("Failed to update camera buffer!");
		return false;
	}

	const XMFLOAT4A camPos = _transform.GetPosition();
	std::memcpy(&_lightingBufferData.camPos[0], &camPos.x, sizeof(float) * 3);
	if (!_lightingBuffer.UpdateBuffer(context, &_lightingBufferData))
	{
		ErrMsg("Failed to update lighting buffer!");
		return false;
	}

	_isDirty = false;
	return true;
}

bool CameraD3D11::BindBuffers(ID3D11DeviceContext *context) const
{
	ID3D11Buffer *const vpmBuffer = GetCameraBuffer();
	context->VSSetConstantBuffers(1, 1, &vpmBuffer);

	ID3D11Buffer *const lightingBuffer = GetLightingBuffer();
	context->PSSetConstantBuffers(0, 1, &lightingBuffer);

	return true;
}


ID3D11Buffer *CameraD3D11::GetCameraBuffer() const
{
	return _cameraBuffer.GetBuffer();
}

ID3D11Buffer *CameraD3D11::GetLightingBuffer() const
{
	return _lightingBuffer.GetBuffer();
}
