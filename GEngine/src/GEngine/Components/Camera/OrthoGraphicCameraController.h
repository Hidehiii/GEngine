#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Components/Camera/OrthoGraphicCamera.h"
#include "GEngine/Events/ApplicationEvent.h"
#include "GEngine/Events/MouseEvent.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Components/Transform/Transform.h"
namespace GEngine
{
	// This class is abandoned
	// Orthographic camera could be set up in the camera component
	// The camera controller will be written in a additional class to control the camera component rather than the orthographic camera
	struct OrthoGraphicCameraBounds
	{
		float Left, Right;
		float Bottom, Top;

		float GetWidth() { return Right - Left; }
		float GetHeight() { return Top - Bottom; }
	};


	class GENGINE_API OrthoGraphicCameraController
	{
	public:
		OrthoGraphicCameraController(float aspectRatio, bool enableRotation = false);

		void OnUpdate();
		void OnEvent(Event& e);

		const OrthoGraphicCamera& GetCamera() const { return m_OrthoGraphicCamera; }
		OrthoGraphicCamera& GetCamera() { return m_OrthoGraphicCamera; }

		void OnResize(float width, float height);

		float GetZoomLevel() const { return m_ZoomLevel; }
		void SetZoomLevel(float level) { m_ZoomLevel = level; CalculateView(); }

		const OrthoGraphicCameraBounds& GetBounds() const { return m_Bounds; }

	private:
		void CalculateView();

		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnWindowResized(WindowResizeEvent& e);
	private:
		float m_AspectRatio;
		float m_ZoomLevel = 1.0f;
		bool m_EnableRotation;
		OrthoGraphicCameraBounds m_Bounds;
		OrthoGraphicCamera m_OrthoGraphicCamera;

		Ref<Transform> m_CameraTransform = CreateRef<Transform>();
		float m_CameraMoveSpeed = 0.8f;
		float m_CameraRotationSpeed = 10.0f;
	};
}


