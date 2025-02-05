#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Object/GameObject.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Components/Component.h"

namespace GEngine
{
	enum class CameraType : int
	{
		OrthoGraphic	= 0,
		Perspective		= 1
	};

	static std::string CameraTypeString[] =
	{
		"OrthoGraphic",
		"Perspective"
	};

	enum class AntiAliasingType
	{
		None	= 0,
		MSAA2x	= 1,
		MSAA4x	= 2,
		MSAA8x	= 3,
		MSAA16x	= 4
	};

	static std::string AntiAliasingTypeString[] =
	{
		"None",
		"2xMSAA",
		"4xMSAA",
		"8xMSAA",
		"16xMSAA"
	};

	class GENGINE_API Camera : public Component
	{
	public:
		Camera();
		Camera(CameraType type);
		Camera(CameraType type, float fov, float aspectRatio, float nearClip, float farClip);
		Camera(const Camera&) = default;

		Matrix4x4	GetProjectionMatrix() { RecalculateProjectionMatrix(); return m_ProjectionMatrix; }
		CameraType	GetCameraType() { return m_CameraType; }
		float		GetAspectRatio() { return m_AspectRatio; }

		void SetViewportSize(uint32_t width, uint32_t height);
		void SetCameraType(CameraType type) { m_CameraType = type;  }
		void SetCameraType(int type) { m_CameraType = CameraType(type); }
		

		static int AntiAliasingTypeToInt(AntiAliasingType type);

		static std::string GetCameraTypeString(CameraType type) { return CameraTypeString[(int)type];  }
		static std::string GetAntiAliasingTypeString(AntiAliasingType type) { return AntiAliasingTypeString[(int)type]; }
	protected: 
		void RecalculateProjectionMatrix();
	public:
		// Orthographic Camera
		void SetOrthoGraphic(float size, float nearClip, float farClip);
		void SetOrthoGraphicSize(float size) { m_OrthoGraphicSize = size; }
		void SetOrthoGraphicNearClip(float nearClip) { m_OrthoGraphicNear = nearClip; }
		void SetOrthoGraphicFarClip(float farClip) { m_OrthoGraphicFar = farClip; }

		float GetOrthoGraphicSize() { return m_OrthoGraphicSize; }
		float GetOrthoGraphicNearClip() { return m_OrthoGraphicNear; }
		float GetOrthoGraphicFarClip() { return m_OrthoGraphicFar; }
		

		// order: LB LT RT RB
		Vector3* GetOrthoGraphicNearClipCoord();
		Vector3* GetOrthoGraphicFarClipCoord();
		// order: NLB NLT NRT NRB FLB FLT FRT FRB
		Vector3* GetOrthoGraphicClipCoord();
	public:
		// Perspective Camera
		void SetPerspective(float fov, float nearClip, float farClip);
		void SetPerspectiveVerticalFOV(float fov) { m_PerspectiveFOV = fov; }
		void SetPerspectiveNearClip(float nearClip) { nearClip = std::max(0.01f, nearClip);  m_PerspectiveNear = nearClip; }
		void SetPerspectiveFarClip(float farClip) { farClip = std::max(m_PerspectiveNear + 0.01f, farClip);  m_PerspectiveFar = farClip; }

		float GetPerspectiveVerticalFOV() { return m_PerspectiveFOV; }
		float GetPerspectiveNearClip() { return m_PerspectiveNear; }
		float GetPerspectiveFarClip() { return m_PerspectiveFar; }
		

		// order: LB LT RT RB
		Vector3* GetPerspectiveNearClipCoord();
		Vector3* GetPerspectiveFarClipCoord();
		// order: NLB NLT NRT NRB FLB FLT FRT FRB
		Vector3* GetPerspectiveClipCoord();
	public:
		// Camera Properties
		// Render order, more positive will display forward
		int RenderOrder = 0;
		// if the camera is the primary camera
		bool isPrimary = false;
		AntiAliasingType m_AntiAliasingType = AntiAliasingType::None;
	public:
		// Orthographic Camera
		float m_OrthoGraphicSize = 10.0f;
		float m_OrthoGraphicNear = -1.0f, m_OrthoGraphicFar = 1.0f;
	public:
		// Perspective Camera
		float m_PerspectiveFOV = 45.0f;
		float m_PerspectiveNear = 0.01f, m_PerspectiveFar = 1000.0f;
	protected:
		Matrix4x4 m_ProjectionMatrix = Matrix4x4(1.0f);
		CameraType m_CameraType = CameraType::OrthoGraphic;
		float m_AspectRatio = 1.0f;
	};

}

