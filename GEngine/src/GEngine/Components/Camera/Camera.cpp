#include "GEpch.h"
#include "Camera.h"
#include "GEngine/Components/Components.h"

#include "GEngine/Renderer/Renderer.h"

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
    Vector3* Camera::GetOrthoGraphicNearClipCoord()
    {
        Vector3* res = new Vector3[4];
        if (m_GameObject)
        {
            auto transform = m_GameObject.GetComponent<Transform>();
            float yf = m_OrthoGraphicSize * 0.5f;
            float xf = yf * m_AspectRatio;
            Vector3 forward = transform.Forward();
            Vector3 up = transform.Up();
            Vector3 right = transform.Right();
            Vector3 f0 = m_OrthoGraphicNear * forward - right * xf - up * yf;
            Vector3 f1 = m_OrthoGraphicNear * forward - right * xf + up * yf;
            Vector3 f2 = m_OrthoGraphicNear * forward + right * xf + up * yf;
            Vector3 f3 = m_OrthoGraphicNear * forward + right * xf - up * yf;

            res[0] = transform.m_Position + f0;
            res[1] = transform.m_Position + f1;
            res[2] = transform.m_Position + f2;
            res[3] = transform.m_Position + f3;
        }
        else
        {
            GE_CORE_ERROR("Camera has no GameObject");
        }
        return res;
    }
    Vector3* Camera::GetOrthoGraphicFarClipCoord()
    {
        Vector3* res = new Vector3[4];
        if (m_GameObject)
        {
            auto transform = m_GameObject.GetComponent<Transform>();
            float yf = m_OrthoGraphicSize * 0.5f;
            float xf = yf * m_AspectRatio;
            Vector3 forward = transform.Forward();
            Vector3 up = transform.Up();
            Vector3 right = transform.Right();
            Vector3 f0 = m_OrthoGraphicFar * forward - right * xf - up * yf;
            Vector3 f1 = m_OrthoGraphicFar * forward - right * xf + up * yf;
            Vector3 f2 = m_OrthoGraphicFar * forward + right * xf + up * yf;
            Vector3 f3 = m_OrthoGraphicFar * forward + right * xf - up * yf;

            res[0] = transform.m_Position + f0;
            res[1] = transform.m_Position + f1;
            res[2] = transform.m_Position + f2;
            res[3] = transform.m_Position + f3;
        }
        else
        {
            GE_CORE_ERROR("Camera has no GameObject");
        }
        return res;
    }
    Vector3* Camera::GetOrthoGraphicClipCoord()
    {
        Vector3* res = new Vector3[8];
        if (m_GameObject)
        {
            auto transform = m_GameObject.GetComponent<Transform>();
            float yf = m_OrthoGraphicSize * 0.5f;
            float xf = yf * m_AspectRatio;
            Vector3 forward = transform.Forward();
            Vector3 up = transform.Up();
            Vector3 right = transform.Right();
            Vector3 f0 = m_OrthoGraphicNear * forward - right * xf - up * yf;
            Vector3 f1 = m_OrthoGraphicNear * forward - right * xf + up * yf;
            Vector3 f2 = m_OrthoGraphicNear * forward + right * xf + up * yf;
            Vector3 f3 = m_OrthoGraphicNear * forward + right * xf - up * yf;

            Vector3 f4 = m_OrthoGraphicFar * forward - right * xf - up * yf;
            Vector3 f5 = m_OrthoGraphicFar * forward - right * xf + up * yf;
            Vector3 f6 = m_OrthoGraphicFar * forward + right * xf + up * yf;
            Vector3 f7 = m_OrthoGraphicFar * forward + right * xf - up * yf;

            res[0] = transform.m_Position + f0;
            res[1] = transform.m_Position + f1;
            res[2] = transform.m_Position + f2;
            res[3] = transform.m_Position + f3;

            res[4] = transform.m_Position + f4;
            res[5] = transform.m_Position + f5;
            res[6] = transform.m_Position + f6;
            res[7] = transform.m_Position + f7;
        }
        else
        {
            GE_CORE_ERROR("Camera has no GameObject");
        }
        return res;
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
    Vector3* Camera::GetPerspectiveNearClipCoord()
    {
        Vector3* res = new Vector3[4];
        if (m_GameObject)
        {
            auto transform = m_GameObject.GetComponent<Transform>();
            float yf = Math::Tan(Math::Radians(m_PerspectiveFOV * 0.5f));
            float xf = yf * m_AspectRatio;
            Vector3 forward = transform.Forward();
            Vector3 up = transform.Up();
            Vector3 right = transform.Right();
            Vector3 f0 = forward - right * xf - up * yf;
            Vector3 f1 = forward - right * xf + up * yf;
            Vector3 f2 = forward + right * xf + up * yf;
            Vector3 f3 = forward + right * xf - up * yf;

            res[0] = transform.m_Position + f0 * m_PerspectiveFar;
            res[1] = transform.m_Position + f1 * m_PerspectiveFar;
            res[2] = transform.m_Position + f2 * m_PerspectiveFar;
            res[3] = transform.m_Position + f3 * m_PerspectiveFar;
        }
        else
        {
            GE_CORE_ERROR("Camera has no GameObject");
        }
        return res;
    }
    Vector3* Camera::GetPerspectiveFarClipCoord()
    {
        Vector3* res = new Vector3[4];
        if (m_GameObject)
        {
            auto transform = m_GameObject.GetComponent<Transform>();
            float yf = Math::Tan(Math::Radians(m_PerspectiveFOV * 0.5f));
            float xf = yf * m_AspectRatio;
            Vector3 forward = transform.Forward();
            Vector3 up = transform.Up();
            Vector3 right = transform.Right();
            Vector3 f0 = forward - right * xf - up * yf;
            Vector3 f1 = forward - right * xf + up * yf;
            Vector3 f2 = forward + right * xf + up * yf;
            Vector3 f3 = forward + right * xf - up * yf;

            res[0] = transform.m_Position + f0 * m_PerspectiveNear;
            res[1] = transform.m_Position + f1 * m_PerspectiveNear;
            res[2] = transform.m_Position + f2 * m_PerspectiveNear;
            res[3] = transform.m_Position + f3 * m_PerspectiveNear;
        }
        else
        {
            GE_CORE_ERROR("Camera has no GameObject");
        }
        return res;
    }
    Vector3* Camera::GetPerspectiveClipCoord()
    {
        Vector3* res = new Vector3[8];
        if (m_GameObject)
        {
            auto transform = m_GameObject.GetComponent<Transform>();
            float yf = Math::Tan(Math::Radians(m_PerspectiveFOV * 0.5f));
            float xf = yf * m_AspectRatio;
            Vector3 forward = transform.Forward();
            Vector3 up = transform.Up();
            Vector3 right = transform.Right();
            Vector3 f0 = forward - right * xf - up * yf;
            Vector3 f1 = forward - right * xf + up * yf;
            Vector3 f2 = forward + right * xf + up * yf;
            Vector3 f3 = forward + right * xf - up * yf;

            res[0] = transform.m_Position + f0 * m_PerspectiveNear;
            res[1] = transform.m_Position + f1 * m_PerspectiveNear;
            res[2] = transform.m_Position + f2 * m_PerspectiveNear;
            res[3] = transform.m_Position + f3 * m_PerspectiveNear;

            res[4] = transform.m_Position + f0 * m_PerspectiveFar;
            res[5] = transform.m_Position + f1 * m_PerspectiveFar;
            res[6] = transform.m_Position + f2 * m_PerspectiveFar;
            res[7] = transform.m_Position + f3 * m_PerspectiveFar;
        }
        else
        {
            GE_CORE_ERROR("Camera has no GameObject");
        }
        return res;
    }
}