#include "RayIntersection.h"

namespace Tk
{
namespace Core
{
namespace Raytracing
{

void IntersectRayTriangle(const Ray& ray, const v3f* triangle, Intersection& isx)
{
    float t = -1.0f;

    // Calculate triangle normal
    v3f normal = Cross(triangle[1] - triangle[0], triangle[2] - triangle[0]);
    float triArea2Inv = 1.0f / Length(normal); // parallelogram area, technically
    Normalize(normal);

    float denom = Dot(normal, ray.dir);
    if (denom == 0.0f)
    {
        // Ray and tri normal are perpendicular - no intersection, avoids div by zero
        return;
    }

    // Calculate triangle plane D value
    float d = Dot(triangle[0], normal);
    t = (d - Dot(normal, ray.origin)) / denom;
    v3f p = ray.origin + t * ray.dir;

    // Check if point lies within triangle + calc barycentric coords
    float subTriArea[3] = {};

    // Edge 0
    v3f areaVec = Cross(p - triangle[2], triangle[1] - triangle[2]);
    if (signbit(Dot(normal, areaVec))) { return; }
    subTriArea[0] = Length(areaVec);

    // Edge 1
    areaVec = Cross(triangle[0] - triangle[2], p - triangle[2]);
    if (signbit(Dot(normal, areaVec))) { return; }
    subTriArea[1] = Length(areaVec);

    // Edge 2
    areaVec = Cross(triangle[1] - triangle[0], p - triangle[0]);
    if (signbit(Dot(normal, areaVec))) { return; }
    subTriArea[2] = Length(areaVec);

    isx.t = t;
    isx.bary[0] = subTriArea[0] * triArea2Inv;
    isx.bary[1] = subTriArea[1] * triArea2Inv;
    isx.bary[2] = subTriArea[2] * triArea2Inv;
}

}
}
}

