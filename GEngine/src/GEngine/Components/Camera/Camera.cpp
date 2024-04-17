#include "GEpch.h"
#include "Camera.h"

namespace GEngine
{
    Camera::Camera()
    {
        RecalculateProjectionMatrix();
    }
    Camera::Camera(CameraType type)
    {
        m_CameraType = type;
        RecalculateProjectionMatrix();
    }
    Camera::Camera(CameraType type, float fov, float aspectRatio, float nearClip, float farClip)
    {
        m_CameraType = type;
        m_PerspectiveFOV = fov;
        m_AspectRatio = aspectRatio;
        m_PerspectiveNear = nearClip;
        m_PerspectiveFar = farClip;
        RecalculateProjectionMatrix();
    }
    void Camera::SetViewportSize(uint32_t width, uint32_t height)
    {
        m_AspectRatio = (float)width / (float)height;
        RecalculateProjectionMatrix();
    }
    void Camera::RecalculateProjectionMatrix()
    {
        switch (m_CameraType)
        {
            case CameraType::OrthoGraphic:
                {
                    m_OrthoGraphicSize = std::max(m_OrthoGraphicSize, 0.001f);
                    float orthoLeft = -m_OrthoGraphicSize * m_AspectRatio * 0.5f;
                    float orthoRight = m_OrthoGraphicSize * m_AspectRatio * 0.5f;
                    float orthoBottom = -m_OrthoGraphicSize * 0.5f;
                    float orthoTop = m_OrthoGraphicSize * 0.5f;
                    m_ProjectionMatrix = Math::Orthographic(orthoLeft, orthoRight, orthoBottom, orthoTop, m_OrthoGraphicNear, m_OrthoGraphicFar);
                    break;
                }
            case CameraType::Perspective:
                {
                    m_PerspectiveNear = std::max(0.01f, m_PerspectiveNear);
                    m_PerspectiveFar = std::max(m_PerspectiveNear, m_PerspectiveFar);
                    m_ProjectionMatrix = Math::Perspective(m_PerspectiveFOV, m_AspectRatio, m_PerspectiveNear, m_PerspectiveFar);
                    break;
                }
        default:
            break;
        }
    }
    void Camera::SetOrthoGraphic(float size, float nearClip, float farClip)
    {
        m_CameraType = CameraType::OrthoGraphic;
        m_OrthoGraphicSize = size;
        m_OrthoGraphicNear = nearClip;
        m_OrthoGraphicFar = farClip;
        RecalculateProjectionMatrix();
    }
    void Camera::SetPerspective(float fov, float nearClip, float farClip)
    {
        m_CameraType = CameraType::Perspective;
        m_PerspectiveFOV = fov;
        nearClip = std::max(0.01f, nearClip);
        m_PerspectiveNear = nearClip;
        farClip = std::max(m_PerspectiveNear, farClip);
        m_PerspectiveFar = farClip;
        RecalculateProjectionMatrix();
    }
}