#include "Camera.h"

using namespace Tk;
using namespace Core;

static const v3f WORLD_UP = v3f(0, 0, 1);
static const float nearPlane = 0.1f;
static const float farPlane = 1000.0f;

void UpdateAxisVectors(Camera* camera)
{
    v3f forward = camera->m_ref - camera->m_eye;
    Normalize(forward);

    v3f right = Cross(forward, WORLD_UP);
    Normalize(right);
    v3f up = Cross(right, forward);
    Normalize(up);

    camera->m_forward = forward;
    camera->m_right = right;
    camera->m_up = up;
}

m4f CameraViewMatrix(const Camera* camera)
{
    m4f view;
    view[0] = v4f(camera->m_right.x, camera->m_up.x, camera->m_forward.x, 0.0f);
    view[1] = v4f(camera->m_right.y, camera->m_up.y, camera->m_forward.y, 0.0f);
    view[2] = v4f(camera->m_right.z, camera->m_up.z, camera->m_forward.z, 0.0f);
    view[3] = v4f(-Dot(camera->m_eye, camera->m_right), -Dot(camera->m_eye, camera->m_up), -Dot(camera->m_eye, camera->m_forward), 1.0f);
    return view;
}

m4f PerspectiveProjectionMatrix(float aspect)
{
    const float tanFov = tanf(fovy * 0.5f);

    m4f proj;
    proj[0][0] = 1.0f / (aspect * tanFov); proj[1][0] = 0.0f;          proj[2][0] = 0.0f;                                             proj[3][0] = 0.0f;
    proj[0][1] = 0.0f;                     proj[1][1] = 1.0f / tanFov; proj[2][1] = 0.0f;                                             proj[3][1] = 0.0f;
    proj[0][2] = 0.0f;                     proj[1][2] = 0.0f;          proj[2][2] = (-nearPlane - farPlane) / (nearPlane - farPlane); proj[3][2] = (2.0f * farPlane * nearPlane) / (nearPlane - farPlane);
    proj[0][3] = 0.0f;                     proj[1][3] = 0.0f;          proj[2][3] = 1.0f;                                             proj[3][3] = 0.0f;
    return proj;
}

inline void PanCamera(Camera* camera, v3f displacement)
{
    camera->m_eye += displacement;
    camera->m_ref += displacement;
}

void PanCameraAlongForward(Camera* camera, float amount)
{
    PanCamera(camera, camera->m_forward * amount);
}

void PanCameraAlongRight(Camera* camera, float amount)
{
    PanCamera(camera, camera->m_right * amount);
}

void PanCameraAlongUp(Camera* camera, float amount)
{
    PanCamera(camera, camera->m_up * amount);
}

void RotateCameraAboutForward(Camera* camera, float rads)
{
    // TODO: rotate right and up about look
}

void RotateCameraAboutUp(Camera* camera, float rads)
{
    const m4f rotMat = RotationMatrix_AxisAngle(camera->m_up, rads);
    camera->m_forward = (rotMat * v4f(camera->m_forward, 0)).xyz();
    camera->m_right   = (rotMat * v4f(camera->m_right, 0)).xyz();
    float refPtDist = Length(camera->m_eye - camera->m_ref);
    camera->m_ref = camera->m_eye + camera->m_forward * refPtDist;
}

void RotateCameraAboutRight(Camera* camera, float rads)
{
    const m4f rotMat = RotationMatrix_AxisAngle(camera->m_right, rads);
    camera->m_forward = (rotMat * v4f(camera->m_forward, 0)).xyz();
    camera->m_up = (rotMat * v4f(camera->m_up, 0)).xyz();
    float refPtDist = Length(camera->m_eye - camera->m_ref);
    camera->m_ref = camera->m_eye + camera->m_forward * refPtDist;
}
