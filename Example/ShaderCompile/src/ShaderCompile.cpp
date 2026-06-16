#include "ShaderCompile.h"

namespace GEngine
{
	ShaderCompile::ShaderCompile()
	{
	}
	void ShaderCompile::OnAttach()
	{
		Ref<Shader> shader = Shader::Create("Assets/Shaders/ExampleShader.shader");
	}
	void ShaderCompile::OnPresent()
	{
	}
	void ShaderCompile::OnRender()
	{
	}
	void ShaderCompile::OnUpdate()
	{
		
	}
	void ShaderCompile::OnImGuiRender()
	{
	}
	void ShaderCompile::OnEvent(Event& e)
	{
		//GE_INFO("Event: {0}", e.ToString());
	}
}