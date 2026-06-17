#include "GEpch.h"
#include "OpenGLImGui.h"
#include "GEngine/Application.h"
#include <glad/glad.h>
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include "ImGui/backends/imgui_impl_glfw.h"
#include "ImGui/backends/imgui_impl_win32.h"
#include "ImGui/backends/imgui_impl_opengl3.h"
#include "GEngine/Graphics/FrameBuffer.h"
#include "Platform/OpenGL/OpenGLCommandBuffer.h"
namespace GEngine
{
	RenderPassSpecification		s_Spec;
	Ref<RenderPass>				s_RenderPass;
	Ref<FrameBuffer>			s_FrameBuffer;

	OpenGLImGui::~OpenGLImGui()
	{
		ImGui_ImplOpenGL3_Shutdown();
	}

	void OpenGLImGui::OnAttach(void* window)
	{
		s_Spec.ColorRTs			= { FRAME_BUFFER_TEXTURE_FORMAT_RGBA8 };
		s_RenderPass			= RenderPass::Create(s_Spec);
		s_FrameBuffer			= FrameBuffer::Create(s_RenderPass, Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight());

		switch (Application::Get().GetConfig()->GetWindowManagerAPI())
		{
		case Config::CONFIG_WINDOW_MANAGER_API_GLFW:
			ImGui_ImplGlfw_InitForOpenGL(((GLFWwindow*)window), true);
			break;
		case Config::CONFIG_WINDOW_MANAGER_API_WIN32:
			ImGui_ImplWin32_Init(window);
			break;
		default:
			GE_CORE_ASSERT(false, "Unknown window manager api");
			break;
		}
		
		ImGui_ImplOpenGL3_Init("#version 450");
	}
	void OpenGLImGui::Begin()
	{
		if ((s_FrameBuffer->GetWidth() != Application::Get().GetWindow().GetWidth() ||
			s_FrameBuffer->GetHeight() != Application::Get().GetWindow().GetHeight()) &&
			(Application::Get().GetWindow().GetWidth() != 0 &&
				Application::Get().GetWindow().GetHeight() != 0))
		{
			s_FrameBuffer = FrameBuffer::Create(s_FrameBuffer, Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight());
		}
		ImGui_ImplOpenGL3_NewFrame();
	}
	void OpenGLImGui::End()
	{
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		s_FrameBuffer->Begin(nullptr);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		s_FrameBuffer->End(nullptr);
	}
	Ref<Texture2D> OpenGLImGui::GetImGuiTexture()
	{
		return s_FrameBuffer->GetColorRT(0);
	}
	Ref<CommandBuffer> OpenGLImGui::GetCommandBuffer()
	{
		return CreateRef<OpenGLCommandBuffer>(COMMAND_BUFFER_TYPE_GRAPHICS);
	}
}
