#include "GEpch.h"
#include "Material.h"
#include "GEngine/Renderer/Renderer.h"
#include "GEngine/Renderer/RendererAPI.h"

#include "Platform/OpenGL/OpenGLMaterial.h"
#include "Platform/Vulkan/VulkanMaterial.h"

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
		case RendererAPI::API::Vulkan: {
			return CreateRef<VulkanMaterial>(shader, name);
			return nullptr;
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