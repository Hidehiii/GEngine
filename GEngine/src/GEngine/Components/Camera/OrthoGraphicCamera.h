#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Math/Matrix4x4.h"
#include "GEngine/Math/Vector3.h"
namespace GEngine
{
	// This class is abandoned
	// Orthographic camera could be set up in the camera component
	class GENGINE_API OrthoGraphicCamera
	{
	public:
		OrthoGraphicCamera(float left, float right, float bottom, float top);

		void SetProjection(float left, float right, float bottom, float top);

		void SetPosition(const Vector3& position) { m_Position = position; RecalculateViewMatrix(); }
		Vector3 GetPosition() const { return m_Position; }

		void SetRotation(Vector3 rotation) { m_Rotation = rotation; RecalculateViewMatrix(); }
		const Vector3 GetRotation() const { return m_Rotation; }

		Matrix4x4 GetProjection() const { return m_ProjectionMatrix; }
		Matrix4x4 GetView() const { return m_ViewMatrix; }
		Matrix4x4 GetViewProjection() const { return m_ViewProjectionMatrix; }

	private:
		void RecalculateViewMatrix();

	private:
		Matrix4x4 m_ProjectionMatrix;
		Matrix4x4 m_ViewMatrix;
		Matrix4x4 m_ViewProjectionMatrix;
		Vector3 m_Position = { 0.0f, 0.0f, 0.0f };
		Vector3 m_Rotation = { 0.0f, 0.0f, 0.0f };
	};
}