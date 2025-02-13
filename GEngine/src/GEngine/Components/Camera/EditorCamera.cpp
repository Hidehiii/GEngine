#include "GEpch.h"
#include "EditorCamera.h"
#include "GEngine/Core/Time.h"
#include "GEngine/Core/Input.h"

namespace GEngine::Editor
{
	EditorCamera::EditorCamera(float fov, float aspectRatio, float nearClip, float farClip)
		: Camera(CameraType::Perspective, fov, aspectRatio, nearClip, farClip)
	{
		UpdateView();
	}

	void EditorCamera::UpdateProjection()
	{
		m_AspectRatio = m_ViewportWidth / m_ViewportHeight;
		RecalculateProjectionMatrix();
	}

	void EditorCamera::UpdateView()
	{
		// m_Yaw = m_Pitch = 0.0f; // Lock the camera's rotation
		m_Position = CalculatePosition();

		Quaternion orientation = GetOrientation();
		m_ViewMatrix = Math::Translate(Matrix4x4(1.0f), m_Position) * Math::ToMatrix4x4(orientation);
		m_ViewMatrix = Math::Inverse(m_ViewMatrix);
	}

	std::pair<float, float> EditorCamera::PanSpeed() const
	{
		float x = std::min(m_ViewportWidth / 1000.0f, 2.4f); // max = 2.4f
		float xFactor = 0.0366f * (x * x) - 0.1778f * x + 0.3021f;

		float y = std::min(m_ViewportHeight / 1000.0f, 2.4f); // max = 2.4f
		float yFactor = 0.0366f * (y * y) - 0.1778f * y + 0.3021f;

		return { xFactor, yFactor };
	}

	float EditorCamera::RotationSpeed() const
	{
		return 0.8f;
	}

	float EditorCamera::ZoomSpeed() const
	{
		float distance = m_Distance * 0.2f;
		distance = std::max(distance, 0.0f);
		float speed = distance * distance;
		speed = std::min(speed, 100.0f); // max speed = 100
		return speed;
	}

	void EditorCamera::OnUpdate()
	{
		if (Input::IsKeyPressed(KeyCode::LeftAlt))
		{
			const Vector2& mouse{ Input::GetMouseX(), Input::GetMouseY() };
			Vector2 delta = (mouse - m_InitialMousePosition) * 0.003f;

			if (Input::IsMouseButtonPressed(MouseCode::ButtonMiddle))
				MousePan(delta);
			else if (Input::IsMouseButtonPressed(MouseCode::ButtonLeft))
				MouseRotate(delta);
			else if (Input::IsMouseButtonPressed(MouseCode::ButtonRight))
				MouseZoom(delta.x);
		}
		else if (Input::IsMouseButtonPressed(MouseCode::ButtonMiddle))
		{
			const Vector2& mouse{ Input::GetMouseX(), Input::GetMouseY() };
			Vector2 delta = (mouse - m_InitialMousePosition) * 0.003f;
			MousePan(delta);
		}
		else if (Input::IsMouseButtonPressed(MouseCode::ButtonRight))
		{
				
			const Vector2& mouse{ Input::GetMouseX(), Input::GetMouseY() };
			Vector2 delta = (mouse - m_InitialMousePosition) * 0.003f;
			float speed = 1;
			if (Input::IsKeyPressed(KeyCode::LeftShift))
			{
				speed *= 3;
			}
			if (Input::IsKeyPressed(KeyCode::W))
			{
				MouseZoom(speed  * 0.003f);
			}
			if (Input::IsKeyPressed(KeyCode::S))
			{
				MouseZoom(-speed * 0.003f);
			}
			if (Input::IsKeyPressed(KeyCode::A))
			{
				MousePan({ speed * 0.003f, 0 });
			}
			if(Input::IsKeyPressed(KeyCode::D))
			{
				MousePan({ -speed * 0.003f, 0 });
			}
			if (Input::IsKeyPressed(KeyCode::Q))
			{
				MousePan({ 0, speed * 0.003f });
			}
			if (Input::IsKeyPressed(KeyCode::E))
			{
				MousePan({ 0, -speed * 0.003f });
			}
			MouseRotate(delta);
		}
		m_InitialMousePosition = { Input::GetMouseX(), Input::GetMouseY() };
		UpdateView();
	}

	void EditorCamera::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(GE_BIND_CLASS_FUNCTION_LAMBDA(EditorCamera::OnMouseScroll));
	}

	bool EditorCamera::OnMouseScroll(MouseScrolledEvent& e)
	{
		float delta = e.GetYOffset() * 0.1f;
		MouseZoom(delta);
		UpdateView();
		return false;
	}

	void EditorCamera::MousePan(const Vector2& delta)
	{
		auto [xSpeed, ySpeed] = PanSpeed();
		m_FocalPoint += -GetRightDirection() * delta.x * xSpeed * m_Distance;
		m_FocalPoint += GetUpDirection() * delta.y * ySpeed * m_Distance;
	}

	void EditorCamera::MouseRotate(const Vector2& delta)
	{
		float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
		m_Yaw += yawSign * delta.x * RotationSpeed();
		m_Pitch += delta.y * RotationSpeed();
	}

	void EditorCamera::MouseZoom(float delta)
	{
		m_Distance -= delta * ZoomSpeed();
		if (m_Distance < 1.0f)
		{
			m_FocalPoint += GetForwardDirection();
			m_Distance = 1.0f;
		}
	}

	Vector3 EditorCamera::GetUpDirection() const
	{
		return Math::Rotate(GetOrientation(), Vector3(0.0f, 1.0f, 0.0f));
	}

	Vector3 EditorCamera::GetRightDirection() const
	{
		return Math::Rotate(GetOrientation(), Vector3(1.0f, 0.0f, 0.0f));
	}

	Vector3 EditorCamera::GetForwardDirection() const
	{
		return Math::Rotate(GetOrientation(), Vector3(0.0f, 0.0f, -1.0f));
	}

	Vector3 EditorCamera::CalculatePosition() const
	{
		return m_FocalPoint - GetForwardDirection() * m_Distance;
	}

	Quaternion EditorCamera::GetOrientation() const
	{
		return Quaternion(Vector3(-m_Pitch, -m_Yaw, 0.0f));
	}

}