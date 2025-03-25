#include "GEpch.h"
#include "OpenGLComputePipeline.h"
#include "GEngine/Graphics/RenderCommand.h"

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
	void OpenGLComputePipeline::Compute(CommandBuffer* cmdBuffer, uint32_t x, uint32_t y, uint32_t z)
	{
		m_Material->Update(cmdBuffer);
		RenderCommand::Compute(cmdBuffer, x, y, z);
	}
}
