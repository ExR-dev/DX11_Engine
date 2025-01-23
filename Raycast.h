#pragma once

#include <DirectXMath.h>
#include <DirectXCollision.h>
//#include <cmath>


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

/*
static bool Raycast(
    const DirectX::XMFLOAT3 &origin, const DirectX::XMFLOAT3 &direction, 
    const DirectX::BoundingOrientedBox &box, float &length)
{
    float // Distances to entry & exit.
        minV = -FLT_MAX,
        maxV = FLT_MAX;
    
    bool RayOBBIntersect(in vec3 rO, in vec3 rD, in int i, out float l, out vec3 p, out vec3 n, out int side)
{
    i *= 5;

    vec3
        rayToCenter = obbShapes[i] - rO,
        nMin = vec3(0),
        nMax = vec3(0);

    for (int a = 0; a < 3; a++)
    { // Check each axis individually.
        vec3 axis = obbShapes[i+2+a];
        float halfLength = obbShapes[i+1][a];

        float 
            distAlongAxis = dot(axis, rayToCenter), // Distance from ray to OBB center along axis.
            f = dot(axis, rD); // Length of direction.

        if (abs(f) > MINVAL)
        { // Ray is not orthogonal to axis.
            vec3 
                tnMin = axis,
                tnMax = axis * -1.0;

            float
                t0 = (distAlongAxis + halfLength) / f,
                t1 = (distAlongAxis - halfLength) / f;

            if (t0 > t1)
            { // Flip intersection order.
                float temp = t0;
                t0 = t1;
                t1 = temp;

                tnMin = tnMax;
                tnMax = axis;
            }

            if (t0 > minV)
            { // Keep the longer entry-point.
                minV = t0;
                nMin = tnMin;
            }
            if (t1 < maxV)
            { // Keep the shorter exit-point.
                maxV = t1;
                nMax = tnMax;
            }
            
            if (minV > maxV)	return false; // Ray misses OBB.
            if (maxV < 0.0)	    return false; // OBB is behind ray.
        }
        else if (-distAlongAxis - halfLength > 0.0 
              || -distAlongAxis + halfLength < 0.0)
        { // Ray is orthogonal to axis but not located between the axis-planes.
            return false;
        }
    }

    // Find the closest positive intersection.
    if (minV > 0.0)
    {
        l = minV;
        n = nMin;
        side = 1;
    }
    else
    {
        l = maxV;
        n = -nMax;
        side = -1;
    }

    p = rO + rD * l;
    n = normalize(n);
    return true;
}

    return false; // TODO: Implement this
}
*/


static bool Raycast(
    const DirectX::XMFLOAT3 &origin, const DirectX::XMFLOAT3 &direction,
    const DirectX::BoundingOrientedBox &box, float &length)
{
    using namespace DirectX;

    // Convert inputs to SIMD-friendly types.
    XMVECTOR rayOrigin = XMLoadFloat3(&origin);
    XMVECTOR rayDirection = XMVector3Normalize(XMLoadFloat3(&direction));
    XMVECTOR boxCenter = XMLoadFloat3(&box.Center);
    XMVECTOR boxExtents = XMLoadFloat3(&box.Extents);
    XMVECTOR boxOrientation = XMLoadFloat4(&box.Orientation);

    XMVECTOR boxRight = { 1, 0, 0 };
    XMVECTOR boxUp = { 0, 1, 0 };
    XMVECTOR boxForward = { 0, 0, 1 };

	boxRight = XMVector3Rotate(boxRight, boxOrientation);
    boxUp = XMVector3Rotate(boxUp, boxOrientation);
    boxForward = XMVector3Rotate(boxForward, boxOrientation);

	XMMATRIX boxRotation = XMMatrixSet(
		XMVectorGetX(boxRight), XMVectorGetY(boxRight), XMVectorGetZ(boxRight), 0,
		XMVectorGetX(boxUp), XMVectorGetY(boxUp), XMVectorGetZ(boxUp), 0,
		XMVectorGetX(boxForward), XMVectorGetY(boxForward), XMVectorGetZ(boxForward), 0,
		0, 0, 0, 1
	);

    float minT = -FLT_MAX;
    float maxT = FLT_MAX;

    for (int axis = 0; axis < 3; ++axis)
    {
        // Extract box axis and compute half-length along this axis.
        XMVECTOR boxAxis = XMVector3Normalize(boxRotation.r[axis]);
        float halfLength = XMVectorGetByIndex(boxExtents, axis);

        // Compute projection of the ray origin onto this axis.
        float distAlongAxis = XMVectorGetX(XMVector3Dot(boxAxis, XMVectorSubtract(boxCenter, rayOrigin)));
        float f = XMVectorGetX(XMVector3Dot(boxAxis, rayDirection));

        if (std::abs(f) > FLT_EPSILON) // Ray is not orthogonal to axis.
        {
            float t0 = (distAlongAxis + halfLength) / f;
            float t1 = (distAlongAxis - halfLength) / f;

            if (t0 > t1) // Flip intersection order.
            {
                std::swap(t0, t1);
            }

            if (t0 > minT)
            {
                minT = t0;
            }
            if (t1 < maxT)
            {
                maxT = t1;
            }

            if (minT > maxT) return false; // Ray misses the OBB.
            if (maxT < 0.0f) return false; // OBB is behind the ray.
        }
        else if (-distAlongAxis - halfLength > 0.0f || -distAlongAxis + halfLength < 0.0f)
        {
            // Ray is orthogonal to axis but outside the OBB bounds.
            return false;
        }
    }

    // Determine the closest valid intersection.
    if (minT > 0.0f)
        length = minT;
    else
        length = maxT;

    return true;
}