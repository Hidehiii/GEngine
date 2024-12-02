#pragma once
#include "GEngine/Renderer/Material.h"
#include "Platform/Vulkan/VulkanShader.h"
#include "Platform/Vulkan/VulkanUniformBuffer.h"

namespace GEngine
{
	class GENGINE_API VulkanMaterial : public Material
	{
	public:
		VulkanMaterial(const Ref<Shader>& shader, const std::string& name = "");
		virtual ~VulkanMaterial() override;

		virtual void UploadData() override;

		virtual void SetIntArray(const std::string& name, int* value, uint32_t count) override;


		virtual Ref<Shader>& GetShader() override { return std::static_pointer_cast<Shader>(m_Shader); }

		virtual void SetShader(const Ref<Shader>& shader) override;

		virtual void SetMatrix4x4(const std::string& name, const Matrix4x4& value) override;

		Ref<VulkanUniformBuffer> GetUniformBuffer() { return m_UniformBuffer; }
	private:
		Ref<VulkanShader>									m_Shader;
		Ref<VulkanUniformBuffer>							m_UniformBuffer;
	};

}

