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
	static Ref<FrameBuffer> s_FrameBuffer;
	FrameBufferSpecification	s_Spec;

	void OpenGLImGui::OnAttach(GLFWwindow* window)
	{
		s_Spec.Attachments = { FrameBufferTextureFormat::RGBA8 };
		s_Spec.Width = 720;
		s_Spec.Height = 720;
		s_FrameBuffer = FrameBuffer::Create(s_Spec);

		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 450");
	}
	void OpenGLImGui::OnDetach()
	{
		ImGui_ImplOpenGL3_Shutdown();
	}
	void OpenGLImGui::Begin()
	{
		ImGui_ImplOpenGL3_NewFrame();
	}
	void OpenGLImGui::End()
	{
		s_FrameBuffer->Begin();
		RenderCommand::SetClearColor({ 0.0f, 0.0f, 0.0f, 0.0f });
		RenderCommand::Clear();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		s_FrameBuffer->End();
	}
	Ref<Texture2D> OpenGLImGui::GetImGuiTexture()
	{
		return s_FrameBuffer->GetColorAttachment(0);
	}
}
