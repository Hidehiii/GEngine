#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Events/MouseEvent.h"
#include "GEngine/Components/Camera/Camera.h"

namespace GEngine::Editor
{
	class GENGINE_API EditorCamera : public Camera
	{
	public:
		EditorCamera() = default;
		EditorCamera(float fov, float aspectRatio, float nearClip, float farClip);

		void OnUpdate();
		void OnEvent(Event& e);

		inline float GetDistance() const { return m_Distance; }
		inline void SetDistance(float distance) { m_Distance = distance; }

		inline void SetViewportSize(float width, float height) { m_ViewportWidth = width; m_ViewportHeight = height; UpdateProjection(); }

		Matrix4x4 GetViewMatrix() const { return m_ViewMatrix; }
		Matrix4x4 GetProjectionMatrix() const { return m_ProjectionMatrix; }
		Matrix4x4 GetViewProjection() const { return m_ProjectionMatrix * m_ViewMatrix; }

		Vector3		GetUpDirection() const;
		Vector3		GetRightDirection() const;
		Vector3		GetForwardDirection() const;
		Vector3		GetPosition() const { return m_Position; }
		Quaternion	GetOrientation() const;

		float GetPitch() const { return m_Pitch; }
		float GetYaw() const { return m_Yaw; }
	private:
		void UpdateProjection();
		void UpdateView();

		bool OnMouseScroll(MouseScrolledEvent& e);

		void MousePan(const Vector2& delta);
		void MouseRotate(const Vector2& delta);
		void MouseZoom(float delta);

		Vector3 CalculatePosition() const;

		std::pair<float, float> PanSpeed() const;
		float					RotationSpeed() const;
		float					ZoomSpeed() const;
	private:

		Matrix4x4 m_ViewMatrix;
		Vector3 m_Position = { 0.0f, 0.0f, 0.0f };
		Vector3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };

		Vector2 m_InitialMousePosition = { 0.0f, 0.0f };

		float m_Distance = 10.0f;
		float m_Pitch = 0.0f, m_Yaw = 0.0f;

		float m_ViewportWidth = 1280, m_ViewportHeight = 720;

	};
}


