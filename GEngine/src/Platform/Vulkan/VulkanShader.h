#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Renderer/Shader.h"

#include <vulkan/vulkan.h>
namespace GEngine
{
	class GENGINE_API VulkanShader : public Shader
	{
	public:
		VulkanShader(const std::string& path);
		VulkanShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
		virtual ~VulkanShader();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual int GetBlendMode() override { return m_BlendType; }
		virtual int GetCullMode() override { return m_CullMode; }
		virtual uint32_t GetBlendSourceFactor() override { return m_BlendSourceFactor; }
		virtual uint32_t GetBlendDestinationFactor() override { return m_BlendDestinationFactor; }

		virtual bool GetEnableDepthWrite() override { return m_EnableDepthWrite; }
		virtual bool GetEnableDepthTest() override { return m_EnableDepthTest; }

		virtual std::vector<ShaderUniform> GetUniforms() override { return m_UniformCache; };
		virtual const std::string& GetShaderName() const override { return m_Name; }
		virtual void SetShaderName(std::string name) override { m_Name = name; }
		virtual std::vector<ShaderUniformTexture2D> GetTexture2D() { return m_Texture2DCache;}
		virtual uint32_t GetTexture2DCount() override { return m_Texture2DCache.size(); }

		virtual std::vector<uint32_t> GetVertexShaderSource() { return m_VulkanSPIRV[ShaderStage::Vertex]; }
		virtual std::vector<uint32_t> GetFragmentShaderSource() { return m_VulkanSPIRV[ShaderStage::Fragment]; }

		virtual void SetInt1(const std::string& name, int value) override;
		virtual void SetIntArray(const std::string& name, int* value, uint32_t count) override;
		virtual void SetFloat1(const std::string& name, float value) override;
		virtual void SetFloat2(const std::string& name, const Vector2& value) override;
		virtual void SetFloat3(const std::string& name, const Vector3& value) override;
		virtual void SetFloat4(const std::string& name, const Vector4& value) override;
		virtual void SetMat4x4(const std::string& name, const Matrix4x4& value) override;
		virtual void SetMat4x4Array(const std::string& name, const Matrix4x4* value, const uint32_t count) override;
	protected:
		virtual void SetMacroBool(std::string& source) override;
		virtual void SetMacroExp(std::string& source) override;
	private:
		std::string ReadFile(const std::string& path);
		std::unordered_map<std::string, std::string> PreProcess(const std::string& source);
		void Compile(std::unordered_map<std::string, std::string>& source);

		void CompileOrGetVulkanBinaries(std::unordered_map<std::string, std::string>& shaderSources);
		void CompileOrGetOpenGLBinaries(const std::unordered_map<std::string, std::string>& shaderSources);
		void Reflect(const std::string stage, const std::vector<uint32_t>& shaderData);
	private:
		std::string											m_FilePath;
		std::string											m_Name;

		std::unordered_map<std::string, std::vector<uint32_t>>	m_VulkanSPIRV;
		std::unordered_map<std::string, std::vector<uint32_t>>	m_OpenGLSPIRV;

		std::unordered_map<std::string, std::string>		m_OpenGLSourceCode;

		std::vector<ShaderUniform>							m_UniformCache;
		std::vector<ShaderUniformTexture2D>					m_Texture2DCache;

		int													m_BlendType;
		int													m_CullMode;
		uint32_t											m_BlendSourceFactor = (uint32_t)VK_BLEND_FACTOR_ONE;
		uint32_t											m_BlendDestinationFactor = (uint32_t)VK_BLEND_FACTOR_ZERO;
		bool												m_EnableDepthWrite = true;
		bool												m_EnableDepthTest = true;

		std::vector<std::pair<std::string, bool>>			m_MacroBools = {
			{ ShaderMacroName::GE_ATTACHMENT_UV_STARTS_AT_TOP, true}
		};
		std::vector<std::pair<std::string, std::string>>	m_MacroExps = {

		};
	};
}


