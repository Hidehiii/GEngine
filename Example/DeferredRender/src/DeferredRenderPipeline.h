#pragma once
#include <GEngine.h>

using namespace GEngine;

class DeferredRenderPipeline :public RenderPipeline
{
public:
	DeferredRenderPipeline();

	virtual ~DeferredRenderPipeline() override;

	virtual void Render(std::vector<Camera>& cameras) override;

	virtual void Present(std::vector<Camera>& cameras) override;
private:
	Ref<FrameBuffer>	m_GBuffer;
};

