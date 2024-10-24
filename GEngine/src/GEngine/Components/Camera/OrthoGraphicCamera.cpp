#include "GEpch.h"
#include "OrthoGraphicCamera.h"
#include "GEngine/Math/Math.h"
#include <glm/gtc/matrix_transform.hpp>

namespace GEngine
{
	OrthoGraphicCamera::OrthoGraphicCamera(float left, float right, float bottom, float top)
		: m_ProjectionMatrix(Math::Orthographic(left, right, bottom, top, -1.0f, 1.0f)), m_ViewMatrix(1.0f)
	{
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}
	void OrthoGraphicCamera::SetProjection(float left, float right, float bottom, float top)
	{
		m_ProjectionMatrix = Math::Orthographic(left, right, bottom, top, -1.0f, 1.0f);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}
	void OrthoGraphicCamera::RecalculateViewMatrix()
	{
		Matrix4x4 transform = Math::Translate(m_Position) *
			Math::Rotate(m_Rotation.x, { 1, 0, 0 }) *
			Math::Rotate(m_Rotation.y, { 0, 1, 0 }) *
			Math::Rotate(m_Rotation.z, { 0, 0, 1 });

		m_ViewMatrix = Math::Inverse(transform);
		m_ViewProjectionMatrix = m_ProjectionMatrix * m_ViewMatrix;
	}
}