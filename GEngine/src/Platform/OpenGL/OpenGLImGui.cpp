#include "GEpch.h"
#include "OpenGLImGui.h"
#include "GEngine/Application.h"
#include "ImGui/backends/imgui_impl_opengl3.h"
#include "ImGui/backends/imgui_impl_glfw.h"
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include "ImGui/backends/imgui_impl_opengl3.cpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
namespace GEngine
{
	void OpenGLImGui::OnAttach(GLFWwindow* window)
	{
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
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
}
