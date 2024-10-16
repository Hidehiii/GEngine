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

		virtual Material_CullMode GetCullMode() override { return m_CullMode; }
		virtual Material_BlendMode GetBlendMode() override { return m_BlendMode; }
		virtual uint32_t GetBlendSourceFactor() override { return m_BlendSourceFactor; }
		virtual uint32_t GetBlendDestinationFactor() override { return m_BlendDestinationFactor; }

		virtual void EnableDepthMask(bool enabled) override { m_EnableDepthMask = enabled; }
		virtual bool GetEnableDepthMask() override { return m_EnableDepthMask; }

		virtual void EnableDepthTest(bool enabled) override { m_EnableDepthTest = enabled; }
		virtual bool GetEnableDepthTest() override { return m_EnableDepthTest; }

		virtual void SetCullMode(Material_CullMode mode) override;
		virtual void SetBlendMode(Material_BlendMode mode, uint32_t source, uint32_t dest) override;

		virtual void SetFloat(const std::string& name, float value) override;
		virtual void SetInt(const std::string& name, int value) override;
		virtual void SetUInt(const std::string& name, uint32_t value) override;
		virtual void SetVector(const std::string& name, const Vector2& value) override;
		virtual void SetVector(const std::string& name, const Vector3& value) override;
		virtual void SetVector(const std::string& name, const Vector4& value) override;
		virtual void SetIntArray(const std::string& name, int* value, uint32_t count) override;
		virtual float GetFloat(const std::string& name) override;
		virtual int GetInt(const std::string& name) override;
		virtual uint32_t GetUInt(const std::string& name) override;
		virtual Vector4 GetVector(const std::string& name) override;

		virtual std::string GetName() override { return m_Name; };
		virtual Ref<Shader>& GetShader() override { return std::static_pointer_cast<Shader>(m_Shader); }

		virtual void SetName(const std::string& name) override;
		virtual void SetShader(const Ref<Shader>& shader) override;

		virtual void SetMatrix4x4(const std::string& name, const Matrix4x4& value) override;

		virtual void SetTexture2D(const std::string& name, const Ref<Texture2D>& texture) override;

		virtual std::vector<ShaderUniform>& GetUniforms() override { return m_Uniforms; }
		Ref<VulkanUniformBuffer> GetUniformBuffer() { return m_UniformBuffer; }
	private:
		ShaderUniform GetUniformByName(const std::string& name) const;
	private:
		Ref<VulkanShader>									m_Shader;
		Ref<VulkanUniformBuffer>							m_UniformBuffer;
		std::string											m_Name;
		Material_BlendMode									m_BlendMode = Material_BlendMode::None;
		uint32_t											m_BlendSourceFactor = 0;
		uint32_t											m_BlendDestinationFactor = 0;
		Material_CullMode									m_CullMode = Material_CullMode::Back;
		Buffer												m_UniformStorageBuffer;
		std::vector<ShaderUniform>							m_Uniforms;
		bool												m_EnableDepthMask = true;
		bool												m_EnableDepthTest = true;
	};

}

