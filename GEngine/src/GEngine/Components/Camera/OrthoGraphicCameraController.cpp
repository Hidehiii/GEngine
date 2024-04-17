#include "GEpch.h"
#include "OrthoGraphicCameraController.h"
#include "GEngine/Core/Time.h"
#include "GEngine/Core/KeyCodes.h"
#include "GEngine/Core/Input.h"

namespace GEngine
{
	OrthoGraphicCameraController::OrthoGraphicCameraController(float aspectRatio, bool enableRotation)
		: m_AspectRatio(aspectRatio), m_EnableRotation(enableRotation), m_OrthoGraphicCamera(m_Bounds.Left, m_Bounds.Right, m_Bounds.Bottom, m_Bounds.Top),
		m_Bounds({ -m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel })
	{

	}
	void OrthoGraphicCameraController::OnUpdate()
	{
		if (Input::IsKeyPressed(KeyCode::A)) {
			m_CameraTransform->Translate(Vector3(-m_CameraMoveSpeed, 0.0f, 0.0f) * Time::GetDeltaTime());
		}
		if (Input::IsKeyPressed(KeyCode::D)) {
			m_CameraTransform->Translate(Vector3(m_CameraMoveSpeed, 0.0f, 0.0f) * Time::GetDeltaTime());
		}
		if (Input::IsKeyPressed(KeyCode::W)) {
			m_CameraTransform->Translate(Vector3(0.0f, m_CameraMoveSpeed, 0.0f) * Time::GetDeltaTime());
		}
		if (Input::IsKeyPressed(KeyCode::S)) {
			m_CameraTransform->Translate(Vector3(0.0f, -m_CameraMoveSpeed, 0.0f) * Time::GetDeltaTime());
		}
		if(m_EnableRotation)
		{
			if (Input::IsKeyPressed(KeyCode::Q)) {
				m_CameraTransform->Rotate(Vector3(0.0f, 0.0f, m_CameraRotationSpeed) * Time::GetDeltaTime());
			}
			if (Input::IsKeyPressed(KeyCode::E)) {
				m_CameraTransform->Rotate(Vector3(0.0f, 0.0f, -m_CameraRotationSpeed) * Time::GetDeltaTime());
			}

			m_OrthoGraphicCamera.SetRotation(Math::EulerAngles(m_CameraTransform->m_Rotation));
		}

		m_OrthoGraphicCamera.SetPosition(m_CameraTransform->GetPosition());
		m_CameraMoveSpeed = m_ZoomLevel;
	}
	void OrthoGraphicCameraController::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(GE_BIND_EVENT_FN(OrthoGraphicCameraController::OnMouseScrolled));
		dispatcher.Dispatch<WindowResizeEvent>(GE_BIND_EVENT_FN(OrthoGraphicCameraController::OnWindowResized));
	}
	void OrthoGraphicCameraController::OnResize(float width, float height)
	{
		m_AspectRatio = width / height;
		CalculateView();
	}
	void OrthoGraphicCameraController::CalculateView()
	{
		m_Bounds = { -m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel };
		m_OrthoGraphicCamera.SetProjection(m_Bounds.Left, m_Bounds.Right, m_Bounds.Bottom, m_Bounds.Top);
	}
	bool OrthoGraphicCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
		m_ZoomLevel -= e.GetYOffset() * 0.25f;
		m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);
		CalculateView();
		return false;
	}
	bool OrthoGraphicCameraController::OnWindowResized(WindowResizeEvent& e)
	{
		m_AspectRatio = (float)e.GetWidth() / (float)e.GetHeight();
		CalculateView();
		return false;
	}
}