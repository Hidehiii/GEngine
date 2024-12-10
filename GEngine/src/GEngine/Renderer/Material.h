#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Core/Buffer.h"
#include "GEngine/Renderer/Shader.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Renderer/Texture.h"



namespace GEngine
{
	
	class GENGINE_API Material
	{
	public:
		static Ref<Material> Create(const Ref<Shader>& shader, const std::string& name = "");
		static Ref<Material> Copy(const Ref<Material>& other, const std::string& name = "");

		bool HasChanged() { return m_HasChanged; }
		void ResetHasChanged() { m_HasChanged = false; }

		virtual ~Material() {}

		virtual void UploadData() = 0;

		virtual CullMode GetCullMode() { return m_CullMode; }
		virtual BlendMode GetBlendMode() { return m_BlendMode; }
		virtual BlendFactor GetBlendColorSourceFactor() { return m_BlendColorSourceFactor; }
		virtual BlendFactor GetBlendAlphaSourceFactor() { return m_BlendAlphaSourceFactor; }
		virtual BlendFactor GetBlendColorDestinationFactor() { return m_BlendColorDestinationFactor; }
		virtual BlendFactor GetBlendAlphaDestinationFactor() { return m_BlendAlphaDestinationFactor; }
		virtual bool IsOpaque() { return m_BlendColorSourceFactor == BlendFactor::ONE && m_BlendColorDestinationFactor == BlendFactor::ZERO
											&& m_BlendAlphaSourceFactor == BlendFactor::ONE && m_BlendAlphaDestinationFactor == BlendFactor::ZERO
			                                && m_BlendMode == BlendMode::None; }

		virtual void EnableDepthWrite(bool enabled)  { m_EnableDepthWrite = enabled; }
		virtual bool GetEnableDepthWrite()  { return m_EnableDepthWrite; }

		virtual void EnableDepthTest(bool enabled)  { m_EnableDepthTest = enabled; }
		virtual bool GetEnableDepthTest()  { return m_EnableDepthTest; }

		virtual void SetCullMode(CullMode mode);
		virtual void SetBlendMode(BlendMode mode, BlendFactor source, BlendFactor dest);
		virtual void SetBlendMode(BlendMode mode, BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha);

		virtual void SetFloat(const std::string& name, float value);
		virtual void SetInt(const std::string& name, int value);
		virtual void SetIntArray(const std::string& name, int* value, uint32_t count) = 0;
		virtual void SetUInt(const std::string& name, uint32_t value);
		virtual void SetVector(const std::string& name, const Vector2& value);
		virtual void SetVector(const std::string& name, const Vector3& value);
		virtual void SetVector(const std::string& name, const Vector4& value);

		virtual float GetFloat(const std::string& name);
		virtual int GetInt(const std::string& name);
		virtual uint32_t GetUInt(const std::string& name);
		virtual Vector4 GetVector(const std::string& name);

		virtual Ref<Shader>& GetShader() = 0;
		virtual std::string GetName() { return m_Name; };

		virtual void SetShader(const Ref<Shader>& shader) = 0;
		virtual void SetName(const std::string& name) { m_Name = name; }

		virtual void SetMatrix4x4(const std::string& name, const Matrix4x4& value) = 0;

		virtual void SetTexture2D(const std::string& name, const Ref<Texture2D>& texture);
		virtual Ref<Texture2D> GetTexture2D(const std::string& name);

		virtual void SetStorageImage2D(const std::string& name, const Ref<StorageImage2D>& storageImage);
		virtual Ref<StorageImage2D> GetStorageImage2D(const std::string& name);

		virtual void SetStorageBuffer(const std::string& name, const Ref<StorageBuffer>& storageBuffer);
		virtual Ref<StorageBuffer> GetStorageBuffer(const std::string& name);

		virtual void SetCubeMap(const std::string& name, const Ref<CubeMap>& cubeMap);
		virtual Ref<CubeMap> GetCubeMap(const std::string& name);

		virtual std::vector<ShaderUniform>& GetUniforms()  { return m_Uniforms; }
		virtual std::vector<ShaderUniformTexture2D>& GetTexture2Ds()  { return m_Texture2D; }
		virtual std::vector<ShaderUniformStorageImage2D>& GetStorageImage2Ds()  { return m_StorageImage2D; }
		virtual std::vector<ShaderUniformStorageBuffer>& GetStorageBuffers() { return m_StorageBuffer; }
		virtual std::vector<ShaderUniformCubeMap>& GetCubeMaps() { return m_CubeMap; }
	protected:
		ShaderUniform GetUniformByName(const std::string& name) const;
		ShaderUniformTexture2D& GetUniformTexture2DByName(const std::string& name);
		ShaderUniformStorageImage2D& GetUniformStorageImage2DByName(const std::string& name);
		ShaderUniformStorageBuffer& GetUniformStorageBufferByName(const std::string& name);
		ShaderUniformCubeMap& GetUniformCubeMapByName(const std::string& name);
	protected:
		bool m_HasChanged = false;

		
		std::string											m_Name;
		BlendMode											m_BlendMode = BlendMode::None;
		CullMode											m_CullMode = CullMode::Back;
		Buffer												m_UniformsBuffer;
		std::vector<ShaderUniform>							m_Uniforms;
		bool												m_EnableDepthWrite = true;
		bool												m_EnableDepthTest = true;
		std::vector<ShaderUniformTexture2D>					m_Texture2D;
		std::vector<ShaderUniformStorageImage2D>			m_StorageImage2D;
		std::vector<ShaderUniformStorageBuffer>				m_StorageBuffer;
		std::vector<ShaderUniformCubeMap>					m_CubeMap;
		BlendFactor											m_BlendColorSourceFactor = BlendFactor::ONE;
		BlendFactor											m_BlendColorDestinationFactor = BlendFactor::ZERO;
		BlendFactor											m_BlendAlphaSourceFactor = BlendFactor::ONE;
		BlendFactor											m_BlendAlphaDestinationFactor = BlendFactor::ZERO;
	};
}


