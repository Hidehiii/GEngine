#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Core/Buffer.h"
#include "GEngine/Math/Math.h"
#include "GEngine/Graphics/Shader.h"


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

		virtual void Update(CommandBuffer* cmdBuffer, const int& pass) = 0;

		virtual void SetEnableDepthWrite(bool enabled, const int& pass);
		virtual bool GetEnableDepthWrite(int pass) { return m_Passes.at(pass).State.DepthWrite; }

		virtual void				SetDepthTestOp(CompareOperation op, const int& pass);
		virtual CompareOperation	GetDepthTestOp(int pass)	{ return m_Passes.at(pass).State.DepthTestOp; }

		virtual void SetCullMode(CullMode mode, int pass);
		virtual void SetBlendMode(BlendMode mode, BlendFactor source, BlendFactor dest, int pass);
		virtual void SetBlendMode(BlendMode modeColor, BlendMode modeAlpha, BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha, int pass);

		virtual CullMode	GetCull(int pass)					{ return m_Passes.at(pass).State.Cull; }
		virtual BlendMode	GetBlendColor(int pass)				{ return m_Passes.at(pass).State.BlendColor; }
		virtual BlendMode	GetBlendAlpha(int pass)				{ return m_Passes.at(pass).State.BlendAlpha; }
		virtual BlendFactor GetBlendColorSrc(int pass)			{ return m_Passes.at(pass).State.BlendColorSrc; }
		virtual BlendFactor GetBlendAlphaSrc(int pass)			{ return m_Passes.at(pass).State.BlendAlphaSrc; }
		virtual BlendFactor GetBlendColorDst(int pass)			{ return m_Passes.at(pass).State.BlendColorDst; }
		virtual BlendFactor GetBlendAlphaDst(int pass)			{ return m_Passes.at(pass).State.BlendAlphaDst; }

		template<typename T>
		void SetConstBufferData(const std::string& name, const T& value)
		{
			WriteConstProperty(name, (const void*)&value);
		}

		template<typename T>
		T GetConstBufferData(const std::string& name)
		{
			return *(T*)ReadConstProperty(name);
		}

		template<typename T>
		void SetReferenceData(const std::string& name, const Ref<T>& value)
		{
			WriteReferenceProperty(name, (void*)&value);
		}

		template<typename T>
		Ref<T> GetReferenceData(const std::string& name)
		{
			return *(Ref<T>*)ReadReferenceProperty(name);
		}

		virtual Ref<Shader>&	GetShader() = 0;
		virtual std::string		GetName() { return m_Name; };

		virtual void SetShader(const Ref<Shader>& shader) = 0;
		virtual void SetName(const std::string& name) { m_Name = name; }

	protected:
		virtual void InitializePassPropertiesMemory();

		virtual void WriteConstProperty(const std::string& name, const void* value);
		virtual const void* ReadConstProperty(const std::string& name);
		virtual void WriteReferenceProperty(const std::string& name, void* ptr);
		virtual void* ReadReferenceProperty(const std::string& name);

	protected:
		std::string																m_Name;
		Ref<Shader>																m_Shader;

		std::vector<ShaderPass>													m_Passes;
	};
}

 
