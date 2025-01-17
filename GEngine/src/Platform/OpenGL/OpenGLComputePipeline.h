#pragma once
#include "GEngine/Compute/ComputePipeline.h"
#include "Platform/OpenGL/OpenGLMaterial.h"

namespace GEngine
{
	class GENGINE_API OpenGLComputePipeline : public ComputePipeline
	{
	public:
		OpenGLComputePipeline(const Ref<Material>& material);
		virtual ~OpenGLComputePipeline() override;
		virtual void Compute(uint32_t x, uint32_t y, uint32_t z) override;
		virtual Ref<Material> GetMaterial() override;
		virtual void SetMaterial(Ref<Material>& material) override;

	private:
		Ref<OpenGLMaterial>		m_Material;
	};
}


