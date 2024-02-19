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
}

void CameraD3D11::MoveLocal(const float amount, const XMFLOAT4A &direction)
{
	_transform.MoveLocal({ 
		direction.x * amount,
		direction.y * amount,
		direction.z * amount,
		0.0f
	});
}


void CameraD3D11::RotatePitch(const float amount)
{
	_transform.RotatePitch(amount);
}

void CameraD3D11::RotateYaw(const float amount)
{
	_transform.RotateYaw(amount);
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

	return true;
}

void CameraD3D11::MoveForward(float amount)
{
}

void CameraD3D11::MoveRight(float amount)
{
}

void CameraD3D11::MoveUp(float amount)
{
}

void CameraD3D11::RotateForward(float amount)
{
}

void CameraD3D11::RotateRight(float amount)
{
}

void CameraD3D11::RotateUp(float amount)
{
}

const XMFLOAT4A &CameraD3D11::GetPosition() const
{
	// TODO: insert return statement here
}

const XMFLOAT4A &CameraD3D11::GetForward() const
{
	// TODO: insert return statement here
}

const XMFLOAT4A &CameraD3D11::GetRight() const
{
	// TODO: insert return statement here
}

const XMFLOAT4A &CameraD3D11::GetUp() const
{
	// TODO: insert return statement here
}

bool CameraD3D11::UpdateInternalConstantBuffer(ID3D11DeviceContext *context) const
{
	const XMFLOAT4X4 viewProjMatrix = GetViewProjectionMatrix();
	if (_cameraBuffer.UpdateBuffer(context, &viewProjMatrix))
	{
		ErrMsg("Failed to update camera buffer!");
		return false;
	}

	return true;
}

ID3D11Buffer *CameraD3D11::GetConstantBuffer() const
{
	return _cameraBuffer.GetBuffer();
}

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
