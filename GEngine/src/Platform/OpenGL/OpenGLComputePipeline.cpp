#include "GEpch.h"
#include "OpenGLComputePipeline.h"
#include "GEngine/Graphics/Graphics.h"

namespace GEngine
{
	OpenGLComputePipeline::OpenGLComputePipeline(const Ref<Material>& material)
	{
		m_Material = std::dynamic_pointer_cast<OpenGLMaterial>(material);
	}
	OpenGLComputePipeline::~OpenGLComputePipeline()
	{
	}
	Ref<Material> OpenGLComputePipeline::GetMaterial()
	{
		return m_Material;
	}
	void OpenGLComputePipeline::SetMaterial(Ref<Material>& material)
	{
		m_Material = std::dynamic_pointer_cast<OpenGLMaterial>(material);
	}
	void OpenGLComputePipeline::Compute(CommandBuffer* cmdBuffer, int pass, uint32_t x, uint32_t y, uint32_t z)
	{
		m_Material->Update(cmdBuffer, pass);
		glDispatchCompute(x, y, z);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}
}
