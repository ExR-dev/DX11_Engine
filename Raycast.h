#pragma once

#include <DirectXMath.h>
#include <DirectXCollision.h>


#define TO_VEC(x)		( *reinterpret_cast<DirectX::XMVECTOR *>(&(x))			)
#define TO_VEC_PTR(x)	(  reinterpret_cast<DirectX::XMVECTOR *>(&(x))			)
#define TO_CONST_VEC(x) ( *reinterpret_cast<const DirectX::XMVECTOR *>(&(x))	)


static bool Raycast(
    const DirectX::XMFLOAT3 &origin, const DirectX::XMFLOAT3 &direction, 
    const DirectX::BoundingBox &box, float &length)
{
	if (box.Contains(XMLoadFloat3(&origin)))
	{
		length = 0.0f;
		return true;
	}

	if (box.Intersects(XMLoadFloat3(&origin), XMLoadFloat3(&direction), length))
	{
		return true;
	}

    return false;
}

static bool Raycast(
    const DirectX::XMFLOAT3 &origin, const DirectX::XMFLOAT3 &direction,
    const DirectX::BoundingOrientedBox &box, float &length)
{
    if (box.Contains(XMLoadFloat3(&origin)))
    {
        length = 0.0f;
        return true;
    }

    if (box.Intersects(XMLoadFloat3(&origin), XMLoadFloat3(&direction), length))
    {
        return true;
    }

    return false;
}