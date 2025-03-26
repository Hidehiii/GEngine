#pragma once

#include "GEngine/Core/Core.h"

namespace GEngine
{
	class GENGINE_API UniformBuffer
	{
	public:
		virtual ~UniformBuffer() {}
		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) = 0;

		static Ref<UniformBuffer> Create(uint32_t size, uint32_t binding);

		
	protected:
		uint32_t m_Binding = 0;
	};
class GENGINE_API UniformBufferDynamic
	{
	public:
		virtual ~UniformBufferDynamic() {}
		virtual void SetData(const void* data, uint32_t size) = 0;

		static Ref<UniformBufferDynamic> Create(uint32_t size, uint32_t count, uint32_t binding, bool global);

		static const std::vector<Ref<UniformBuffer>>&	GetGlobalUniforms()			{ return s_GlobalUniforms; }
		static const std::vector<uint32_t>&				GetGlobalUniformOffsets()	{ return s_GlobalUniformOffsets; }
	protected:
		void UpdateGlobalUniformOffset();
	protected:
		uint32_t	m_Aligment = 0;
		uint32_t	m_Offset = 0;
		uint32_t	m_TotalSize = 0;
		uint32_t	m_Binding = 0;
		uint32_t	m_GlobalIndex = -1;
	private:
		static std::vector<Ref<UniformBuffer>>		s_GlobalUniforms;
		static std::vector<uint32_t>				s_GlobalUniformOffsets;
	};}



