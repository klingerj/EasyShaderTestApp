#pragma once

#include "CoreDefines.h"
#include "Math/VectorTypes.h"

namespace Tk
{
namespace Core
{
namespace Raytracing
{

struct Ray
{
    v3f origin;
    v3f dir;
};

struct Intersection
{
    float bary[3];
    float t;
    uint32 hitTri;

    void InitInvalid()
    {
        bary[0] = 0.0f;
        bary[1] = 0.0f;
        bary[2] = 0.0f;
        t = -1.0f;
        hitTri = MAX_UINT32;
    }
};

TINKER_API void IntersectRayTriangle(const Ray& ray, const v3f* triangle, Intersection& isx);
//void IntersectRayCube(const Ray& ray, const Box& box);

}
}
}
