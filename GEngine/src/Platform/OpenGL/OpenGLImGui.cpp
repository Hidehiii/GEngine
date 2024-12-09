#include "GEpch.h"
#include "OpenGLImGui.h"
#include "GEngine/Application.h"
#include "ImGui/backends/imgui_impl_opengl3.h"
#include "ImGui/backends/imgui_impl_glfw.h"
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include "ImGui/backends/imgui_impl_opengl3.cpp"
#include "GEngine/Renderer/FrameBuffer.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
namespace GEngine
{
	Ref<FrameBuffer>			s_FrameBuffer;
	FrameBufferSpecification	s_Spec;

	void OpenGLImGui::OnAttach(GLFWwindow* window)
	{
		s_Spec.Attachments	= { FrameBufferTextureFormat::RGBA8 };
		s_Spec.Width		= Application::Get().GetWindow().GetWidth();
		s_Spec.Height		= Application::Get().GetWindow().GetHeight();
		s_FrameBuffer		= FrameBuffer::Create(s_Spec);

		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 450");
	}
	void OpenGLImGui::OnDetach()
	{
		ImGui_ImplOpenGL3_Shutdown();
	}
	void OpenGLImGui::Begin()
	{
		if ((s_Spec.Width != Application::Get().GetWindow().GetWidth() ||
			s_Spec.Height != Application::Get().GetWindow().GetHeight()) &&
			(Application::Get().GetWindow().GetWidth() != 0 &&
				Application::Get().GetWindow().GetHeight() != 0))
		{
			s_Spec.Width = Application::Get().GetWindow().GetWidth();
			s_Spec.Height = Application::Get().GetWindow().GetHeight();
			s_FrameBuffer = FrameBuffer::Recreate(s_FrameBuffer, s_Spec.Width, s_Spec.Height);
		}
		ImGui_ImplOpenGL3_NewFrame();
	}
	void OpenGLImGui::End()
	{
		RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });
		s_FrameBuffer->Begin();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		s_FrameBuffer->End();
	}
	Ref<Texture2D> OpenGLImGui::GetImGuiTexture()
	{
		return s_FrameBuffer->GetColorAttachment(0);
	}
}
