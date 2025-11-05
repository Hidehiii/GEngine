#include "ShaderCompile.h"

namespace GEngine
{
	ShaderCompile::ShaderCompile()
	{
	}
	void ShaderCompile::OnAttach()
	{
		GE_TRACE("{}", sizeof(void*));
		GE_ASSERT(false, "");
		Ref<Material> mat = Material::Create("Assets/Shaders/ExampleShader.shader", "ExampleShader");
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
	}
}