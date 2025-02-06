#include "GEpch.h"
#include "OpenGLComputePipeline.h"
#include "GEngine/Compute/ComputeCommand.h"

namespace GEngine
{
	OpenGLComputePipeline::OpenGLComputePipeline(const Ref<Material>& material)
	{
		m_Material = std::dynamic_pointer_cast<OpenGLMaterial>(material);
	}
	OpenGLComputePipeline::~OpenGLComputePipeline()
	{
	}
	void OpenGLComputePipeline::Compute(uint32_t x, uint32_t y, uint32_t z)
	{
		m_Material->Update();
		ComputeCommand::Compute(x, y, z);
	}
	Ref<Material> OpenGLComputePipeline::GetMaterial()
	{
		return m_Material;
	}
	void OpenGLComputePipeline::SetMaterial(Ref<Material>& material)
	{
		m_Material = std::dynamic_pointer_cast<OpenGLMaterial>(material);
	}
}
