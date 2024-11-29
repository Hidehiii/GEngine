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

		virtual MaterialCullMode GetCullMode() override { return m_CullMode; }
		virtual MaterialBlendMode GetBlendMode() override { return m_BlendMode; }
		virtual uint32_t GetBlendSourceFactor() override { return m_BlendSourceFactor; }
		virtual uint32_t GetBlendDestinationFactor() override { return m_BlendDestinationFactor; }
		virtual bool IsOpaque() override { return (VkBlendFactor)m_BlendSourceFactor == VK_BLEND_FACTOR_ONE && (VkBlendFactor)m_BlendDestinationFactor == VK_BLEND_FACTOR_ZERO; }

		virtual void EnableDepthWrite(bool enabled) override { m_EnableDepthWrite = enabled; }
		virtual bool GetEnableDepthWrite() override { return m_EnableDepthWrite; }

		virtual void EnableDepthTest(bool enabled) override { m_EnableDepthTest = enabled; }
		virtual bool GetEnableDepthTest() override { return m_EnableDepthTest; }

		virtual void SetCullMode(MaterialCullMode mode) override;
		virtual void SetBlendMode(MaterialBlendMode mode, uint32_t source, uint32_t dest) override;

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
		virtual Ref<Texture2D> GetTexture2D(const std::string& name) override { return GetUniformTexture2DByName(name).Texture; }

		virtual void SetStorageImage2D(const std::string& name, const Ref<StorageImage2D>& storageImage) override;
		virtual Ref<StorageImage2D> GetStorageImage2D(const std::string& name) override { return GetUniformStorageImage2DByName(name).Image; }

		virtual std::vector<ShaderUniform>& GetUniforms() override { return m_Uniforms; }
		Ref<VulkanUniformBuffer> GetUniformBuffer() { return m_UniformBuffer; }
		virtual std::vector<ShaderUniformTexture2D>& GetGetTexture2Ds() override { return m_Texture2D; }
	private:
		ShaderUniform GetUniformByName(const std::string& name) const;
		ShaderUniformTexture2D& GetUniformTexture2DByName(const std::string& name);
		ShaderUniformStorageImage2D& GetUniformStorageImage2DByName(const std::string& name);
	private:
		Ref<VulkanShader>									m_Shader;
		Ref<VulkanUniformBuffer>							m_UniformBuffer;
		std::string											m_Name;
		MaterialBlendMode									m_BlendMode = MaterialBlendMode::None;
		uint32_t											m_BlendSourceFactor = (uint32_t)VK_BLEND_FACTOR_ONE;
		uint32_t											m_BlendDestinationFactor = (uint32_t)VK_BLEND_FACTOR_ZERO;
		MaterialCullMode									m_CullMode = MaterialCullMode::Back;
		Buffer												m_UniformStorageBuffer;
		std::vector<ShaderUniform>							m_Uniforms;
		bool												m_EnableDepthWrite = true;
		bool												m_EnableDepthTest = true;
		std::vector<ShaderUniformTexture2D>					m_Texture2D;
		std::vector<ShaderUniformStorageImage2D>			m_StorageImage2D;
	};

}

