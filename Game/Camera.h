#pragma once

#include "Math/VectorTypes.h"

typedef struct camera_data
{
    v3f m_eye;
    v3f m_ref;
    v3f m_forward;
    v3f m_right;
    v3f m_up;
} Camera;

static m4f g_projMat = m4f(1.0f);

static const float fovy = 0.785398f;

void UpdateAxisVectors(Camera* camera);
m4f CameraViewMatrix(const Camera* camera);
m4f PerspectiveProjectionMatrix(float aspect);

void PanCameraAlongForward(Camera* camera, float amount);
void PanCameraAlongRight(Camera* camera, float amount);
void PanCameraAlongUp(Camera* camera, float amount);

void RotateCameraAboutForward(Camera* camera, float rads);
void RotateCameraAboutUp(Camera* camera, float rads);
void RotateCameraAboutRight(Camera* camera, float rads);
