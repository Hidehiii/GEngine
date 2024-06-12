#include "GEpch.h"
#include "Material.h"
#include "GEngine/Renderer/Renderer.h"
#include "GEngine/Renderer/RendererAPI.h"

#include "Platform/OpenGL/OpenGLMaterial.h"


namespace GEngine
{
    Ref<Material> Material::Create(const Ref<Shader>& shader, const std::string& name)
    {
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None: {
			GE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		}
		case RendererAPI::API::OpenGL: {
			return CreateRef<OpenGLMaterial>(shader, name);
		}
		}

		GE_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }
    Ref<Material> Material::Copy(const Ref<Material>& other, const std::string& name)
    {
        return nullptr;;
    }
}