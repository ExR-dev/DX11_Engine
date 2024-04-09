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
    const DirectX::XMFLOAT3 
        invDir = DirectX::XMFLOAT3(1.0f / direction.x, 1.0f / direction.y, 1.0f / direction.z),
        boxMin = DirectX::XMFLOAT3(box.Center.x - box.Extents.x, box.Center.y - box.Extents.y, box.Center.z - box.Extents.z),
        boxMax = DirectX::XMFLOAT3(box.Center.x + box.Extents.x, box.Center.y + box.Extents.y, box.Center.z + box.Extents.z);

    const float
		tx1 = (boxMin.x - origin.x) * invDir.x,
		tx2 = (boxMax.x - origin.x) * invDir.x;

    float
		tmin = min(tx1, tx2),
		tmax = max(tx1, tx2);

    const float
		ty1 = (boxMin.y - origin.y) * invDir.y,
		ty2 = (boxMax.y - origin.y) * invDir.y;

    tmin = max(tmin, min(ty1, ty2));
    tmax = min(tmax, max(ty1, ty2));

    const float
		tz1 = (boxMin.z - origin.z) * invDir.z,
		tz2 = (boxMax.z - origin.z) * invDir.z;

    tmin = max(tmin, min(tz1, tz2));
    tmax = min(tmax, max(tz1, tz2));

    if (!((tmax >= max(0.0, tmin)) && (tmin < FLT_MAX)))
        return false;

    length = (tmin > 0.0) ? tmin : tmax;
    return true;
}