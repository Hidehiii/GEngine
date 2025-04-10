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
		static Ref<Material> Create(const Ref<Shader>& shader, const std::string& name = "");
		static Ref<Material> Create(const std::string& shaderPath, const std::string& name = "");
		static Ref<Material> Copy(const Ref<Material>& other, const std::string& name = "");
		virtual ~Material() {}

		virtual void Update(CommandBuffer* cmdBuffer) = 0;
		virtual void Update(CommandBuffer* cmdBuffer, const std::string& pass) = 0;

		virtual CullMode	GetCull()					{ return m_CullMode; }
		virtual BlendMode	GetBlendColor()				{ return m_BlendModeColor; }
		virtual BlendMode	GetBlendAlpha()				{ return m_BlendModeAlpha; }
		virtual BlendFactor GetBlendColorSrc()			{ return m_BlendColorSourceFactor; }
		virtual BlendFactor GetBlendAlphaSrc()			{ return m_BlendAlphaSourceFactor; }
		virtual BlendFactor GetBlendColorDst()			{ return m_BlendColorDestinationFactor; }
		virtual BlendFactor GetBlendAlphaDst()			{ return m_BlendAlphaDestinationFactor; }

		virtual void EnableDepthWrite(bool enabled)		{ m_EnableDepthWrite = enabled; }
		virtual bool IsEnableDepthWrite()				{ return m_EnableDepthWrite; }

		virtual void				SetDepthTestOp(CompareOperation op)		{ m_DepthTestOperation = op; }
		virtual CompareOperation	GetDepthTestOp()						{ return m_DepthTestOperation; }

		virtual void SetCullMode(CullMode mode);
		virtual void SetBlendMode(BlendMode mode, BlendFactor source, BlendFactor dest);
		virtual void SetBlendMode(BlendMode modeColor, BlendMode modeAlpha, BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha);

		virtual CullMode	GetCull(const std::string& pass)					{ return m_RenderStates[pass].Cull; }
		virtual BlendMode	GetBlendColor(const std::string& pass)				{ return m_RenderStates[pass].BlendColor; }
		virtual BlendMode	GetBlendAlpha(const std::string& pass)				{ return m_RenderStates[pass].BlendAlpha; }
		virtual BlendFactor GetBlendColorSrc(const std::string& pass)			{ return m_RenderStates[pass].BlendColorSrc; }
		virtual BlendFactor GetBlendAlphaSrc(const std::string& pass)			{ return m_RenderStates[pass].BlendAlphaSrc; }
		virtual BlendFactor GetBlendColorDst(const std::string& pass)			{ return m_RenderStates[pass].BlendColorDst; }
		virtual BlendFactor GetBlendAlphaDst(const std::string& pass)			{ return m_RenderStates[pass].BlendAlphaDst; }

		virtual void EnableDepthWrite(bool enabled, const std::string& pass)	{ m_RenderStates[pass].DepthWrite = enabled; }
		virtual bool IsEnableDepthWrite(const std::string& pass)				{ return m_RenderStates[pass].DepthWrite; }

		virtual void				SetDepthTestOp(CompareOperation op, const std::string& pass)	{ m_RenderStates[pass].DepthTestOp = op; }
		virtual CompareOperation	GetDepthTestOp(const std::string& pass)							{ return m_RenderStates[pass].DepthTestOp; }

		virtual void SetCullMode(CullMode mode, const std::string& pass)		{ m_RenderStates[pass].Cull = mode; }
		virtual void SetBlendMode(BlendMode mode, BlendFactor source, BlendFactor dest, const std::string& pass);
		virtual void SetBlendMode(BlendMode modeColor, BlendMode modeAlpha, BlendFactor srcColor, BlendFactor dstColor, BlendFactor srcAlpha, BlendFactor dstAlpha, const std::string& pass);

		virtual void SetFloat(const std::string& name, float value);
		virtual void SetInt(const std::string& name, int value);
		virtual void SetIntArray(const std::string& name, int* value, uint32_t count) = 0;
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
		virtual void					SetTexture2D(const int index, const Ref<Texture2D>& texture);
		virtual const Ref<Texture2D>	GetTexture2D(const std::string& name);

		virtual void						SetStorageImage2D(const std::string& name, const Ref<StorageImage2D>& storageImage);
		virtual void						SetStorageImage2D(const int index, const Ref<StorageImage2D>& storageImage);
		virtual const Ref<StorageImage2D>	GetStorageImage2D(const std::string& name);

		virtual void						SetStorageBuffer(const std::string& name, const Ref<StorageBuffer>& storageBuffer);
		virtual void						SetStorageBuffer(const int index, const Ref<StorageBuffer>& storageBuffer);
		virtual const Ref<StorageBuffer>	GetStorageBuffer(const std::string& name);

		virtual void				SetCubeMap(const std::string& name, const Ref<CubeMap>& cubeMap);
		virtual void				SetCubeMap(const int index, const Ref<CubeMap>& cubeMap);
		virtual void				SetCubeMap(const std::string& name, uint32_t width, uint32_t height, CubeMap::CubeMapFace face, const Ref<Texture2D>& texture);
		virtual void				SetCubeMap(const int index, uint32_t width, uint32_t height, CubeMap::CubeMapFace face, const Ref<Texture2D>& texture);
		virtual const Ref<CubeMap>	GetCubeMap(const std::string& name);

		virtual void								SetTexture2DArray(const std::string& name, uint32_t width, uint32_t height, const int layer, const Ref<Texture2D>& texture);
		virtual void								SetTexture2DArray(const int index, uint32_t width, uint32_t height, const int layer, const Ref<Texture2D>& texture);
		virtual void								SetTexture2DArray(const std::string& name, const Ref<Texture2DArray>& array);
		virtual void								SetTexture2DArray(const int index, const Ref<Texture2DArray>& array);
		virtual const Ref<Texture2DArray>			GetTexture2DArray(const std::string& name);

		virtual const std::vector<ShaderUniform>&					GetUniforms()			{ return m_Uniforms; }
		virtual const std::vector<ShaderUniformTexture2D>&			GetTexture2Ds()			{ return m_Texture2D; }
		virtual const std::vector<ShaderUniformStorageImage2D>&		GetStorageImage2Ds()	{ return m_StorageImage2D; }
		virtual const std::vector<ShaderUniformStorageBuffer>&		GetStorageBuffers()		{ return m_StorageBuffer; }
		virtual const std::vector<ShaderUniformCubeMap>&			GetCubeMaps()			{ return m_CubeMap; }
		virtual const std::vector<ShaderUniformTexture2DArray>&		GetTexture2DArrays()	{ return m_Texture2DArray; }
	protected:
		ShaderUniform					GetUniformByName(const std::string& name) const;
		ShaderUniformTexture2D&			GetUniformTexture2DByName(const std::string& name);
		ShaderUniformStorageImage2D&	GetUniformStorageImage2DByName(const std::string& name);
		ShaderUniformStorageBuffer&		GetUniformStorageBufferByName(const std::string& name);
		ShaderUniformCubeMap&			GetUniformCubeMapByName(const std::string& name);
		ShaderUniformTexture2DArray&	GetUniformTexture2DArrayByName(const std::string& name);
	protected:
		std::string											m_Name;
		BlendMode											m_BlendModeColor = BlendMode::None;
		BlendMode											m_BlendModeAlpha = BlendMode::None;
		CullMode											m_CullMode = CullMode::Back;
		Buffer												m_UniformsBuffer;
		std::vector<ShaderUniform>							m_Uniforms;
		bool												m_EnableDepthWrite = true;
		CompareOperation									m_DepthTestOperation = CompareOperation::LessEqual;
		std::vector<ShaderUniformTexture2D>					m_Texture2D;
		std::vector<ShaderUniformStorageImage2D>			m_StorageImage2D;
		std::vector<ShaderUniformStorageBuffer>				m_StorageBuffer;
		std::vector<ShaderUniformCubeMap>					m_CubeMap;
		std::vector<ShaderUniformTexture2DArray>			m_Texture2DArray;
		BlendFactor											m_BlendColorSourceFactor = BlendFactor::ONE;
		BlendFactor											m_BlendColorDestinationFactor = BlendFactor::ZERO;
		BlendFactor											m_BlendAlphaSourceFactor = BlendFactor::ONE;
		BlendFactor											m_BlendAlphaDestinationFactor = BlendFactor::ZERO;

		std::unordered_map<std::string, RenderState>		m_RenderStates;
	};
}


