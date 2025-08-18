#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Core/Buffer.h"
#include "GEngine/Graphics/Shader.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Graphics/Texture.h"



namespace GEngine
{
	
	class CommandBuffer;

	class GENGINE_API Material
	{
	public:
		static Ref<Material> Create(const Ref<Shader>& shader, const std::string& name);
		static Ref<Material> Create(const std::string& shaderPath, const std::string& name);
		static Ref<Material> Copy(const Ref<Material>& other, const std::string& name);
		virtual ~Material() {}

		virtual void Update(CommandBuffer* cmdBuffer, const std::string& pass) = 0;

		virtual void SetEnableDepthWrite(bool enabled, const int& pass);
		virtual bool GetEnableDepthWrite(const int& pass) { return m_Passes.at(pass).State.DepthWrite; }

		virtual void				SetDepthTestOp(CompareOperation op, const int& pass);
		virtual CompareOperation	GetDepthTestOp(const int& pass)	{ return m_Passes.at(pass).State.DepthTestOp; }

		virtual void SetCullMode(CullMode mode, const int& pass);
		virtual void SetBlendMode(BlendMode mode, BlendFactor source, BlendFactor dest, const int& pass);
		virtual void SetBlendMode(BlendMode modeColor, BlendMode modeAlpha, BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha, const int& pass);

		virtual CullMode	GetCull(const int& pass)					{ return m_Passes.at(pass).State.Cull; }
		virtual BlendMode	GetBlendColor(const int& pass)				{ return m_Passes.at(pass).State.BlendColor; }
		virtual BlendMode	GetBlendAlpha(const int& pass)				{ return m_Passes.at(pass).State.BlendAlpha; }
		virtual BlendFactor GetBlendColorSrc(const int& pass)			{ return m_Passes.at(pass).State.BlendColorSrc; }
		virtual BlendFactor GetBlendAlphaSrc(const int& pass)			{ return m_Passes.at(pass).State.BlendAlphaSrc; }
		virtual BlendFactor GetBlendColorDst(const int& pass)			{ return m_Passes.at(pass).State.BlendColorDst; }
		virtual BlendFactor GetBlendAlphaDst(const int& pass)			{ return m_Passes.at(pass).State.BlendAlphaDst; }

		virtual void SetFloat(const std::string& name, float value);
		virtual void SetInt(const std::string& name, int value);
		virtual void SetUInt(const std::string& name, uint32_t value);
		virtual void SetVector(const std::string& name, const Vector2& value);
		virtual void SetVector(const std::string& name, const Vector3& value);
		virtual void SetVector(const std::string& name, const Vector4& value);
		virtual void SetMatrix4x4(const std::string& name, const Matrix4x4& value);

		virtual float		GetFloat(const std::string& name);
		virtual int			GetInt(const std::string& name);
		virtual uint32_t	GetUInt(const std::string& name);
		virtual Vector4		GetVector(const std::string& name);

		virtual Ref<Shader>&	GetShader() = 0;
		virtual std::string		GetName() { return m_Name; };

		virtual void SetShader(const Ref<Shader>& shader) = 0;
		virtual void SetName(const std::string& name) { m_Name = name; }
		

		virtual void					SetTexture2D(const std::string& name, const Ref<Texture2D>& texture);
		virtual const Ref<Texture2D>	GetTexture2D(const std::string& name);

		virtual void						SetStorageImage2D(const std::string& name, const Ref<StorageImage2D>& storageImage);
		virtual const Ref<StorageImage2D>	GetStorageImage2D(const std::string& name);

		virtual void						SetStorageBuffer(const std::string& name, const Ref<StorageBuffer>& storageBuffer);
		virtual const Ref<StorageBuffer>	GetStorageBuffer(const std::string& name);

		virtual void				SetCubeMap(const std::string& name, const Ref<CubeMap>& cubeMap);
		virtual const Ref<CubeMap>	GetCubeMap(const std::string& name);

		virtual void								SetTexture2DArray(const std::string& name, const Ref<Texture2DArray>& array);
		virtual const Ref<Texture2DArray>			GetTexture2DArray(const std::string& name);

	protected:
		virtual void InitializePropertiesMemory();

		virtual void WriteConstProperty(const std::string& name, const void* value);
		virtual void WriteReferenceProperty(const std::string& name, void* ptr);

	protected:
		std::string																m_Name;

		Buffer																	m_PropertiesMemory;
		std::unordered_map<std::string, void*>									m_Properties;
		std::vector<ShaderPass>													m_Passes;
	};
}


