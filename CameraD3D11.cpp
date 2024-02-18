#include "CameraD3D11.h"

#include "ErrMsg.h"


void CameraD3D11::MoveInDirection(const float amount, const DirectX::XMFLOAT3 &direction)
{
	_transform.MoveLocal({ 
		direction.x * amount,
		direction.y * amount,
		direction.z * amount
	});
}

void CameraD3D11::RotateAroundAxis(float amount, const DirectX::XMFLOAT3 &axis)
{
		_transform.RotateLocal(amount, axis);
}
