#include "ShaderCompile.h"

namespace GEngine
{
	ShaderCompile::ShaderCompile()
	{
	}
	void ShaderCompile::OnAttach()
	{
		Ref<Shader> shader = Shader::Create("Assets/Shaders/ExampleShader.shader");
		Ref<Material> material = Material::Create(shader, "ExampleMaterial");

		material->SetConstant<float>("_SomeFloat", 1.0f);
		float x = material->GetConstant<float>("_SomeFloat");
		GE_INFO("Material constant _SomeFloat: {0}", x);

		struct MyStruct
		{
			float a;
			float b;
		};  // same define with the struct in shader
		material->SetConstant<MyStruct>("_MyStruct", { 1.0f, 2.0f });
		MyStruct myStruct = material->GetConstant<MyStruct>("_MyStruct");
		GE_INFO("Material constant _MyStruct a: {0}, b: {1}", myStruct.a, myStruct.b);
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