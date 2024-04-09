#pragma once

#include <DirectXMath.h>
#include <DirectXCollision.h>


#define TO_VEC(x)		( *reinterpret_cast<DirectX::XMVECTOR *>(&(x))			)
#define TO_VEC_PTR(x)	(  reinterpret_cast<DirectX::XMVECTOR *>(&(x))			)
#define TO_CONST_VEC(x) ( *reinterpret_cast<const DirectX::XMVECTOR *>(&(x))	)


static bool Raycast(
    const DirectX::XMFLOAT4A &origin, const DirectX::XMFLOAT4A &direction, 
    const DirectX::BoundingBox &box, float &length)
{
    const DirectX::XMFLOAT3 invDir = DirectX::XMFLOAT3(1.0f / direction.x, 1.0f / direction.y, 1.0f / direction.z);

    float tx1 = (aabbShapes[i].x - origin.x) * invDir.x;
    float tx2 = (aabbShapes[i+1].x - origin.x) * invDir.x;

    float tmin = min(tx1, tx2);
    float tmax = max(tx1, tx2);

    float ty1 = (aabbShapes[i].y - origin.y) * invDir.y;
    float ty2 = (aabbShapes[i+1].y - origin.y) * invDir.y;

    tmin = max(tmin, min(ty1, ty2));
    tmax = min(tmax, max(ty1, ty2));

    float tz1 = (aabbShapes[i].z - origin.z) * invDir.z;
    float tz2 = (aabbShapes[i+1].z - origin.z) * invDir.z;

    tmin = max(tmin, min(tz1, tz2));
    tmax = min(tmax, max(tz1, tz2));

    if (!((tmax >= max(0.0, tmin)) && (tmin < MAXVAL)))
        return false;

    length = (tmin > 0.0) ? tmin : tmax;
    return true;
}