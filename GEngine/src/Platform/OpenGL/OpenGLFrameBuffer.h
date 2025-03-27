#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/FrameBuffer.h"
#include "OpenGLTexture2D.h"
#include "GEngine/Math/Math.h"
#include "Platform/OpenGL/OpenGLRenderPass.h"

namespace GEngine
{
	class GENGINE_API OpenGLFrameBuffer : public FrameBuffer
	{
	public:
		OpenGLFrameBuffer(const Ref<RenderPass>& renderPass, uint32_t width, uint32_t height);
		OpenGLFrameBuffer(const Ref<FrameBuffer>& buffer, uint32_t width, uint32_t height);
		virtual ~OpenGLFrameBuffer() override;

		virtual void Begin(CommandBuffer* cmdBuffer) override;
		virtual void End(CommandBuffer* cmdBuffer) override;

		virtual void SetRenderPassOperation(const RenderPassOperation& op) override;

		virtual int								GetColorRTCount() override { return m_ColorAttachments.size(); }
		virtual int								GetRTCount() override { return m_DepthStencilAttachment == 0 ? m_ColorAttachments.size() : m_ColorAttachments.size() + 1; };
		virtual const FrameBufferSpecification& GetSpecification() const override { return m_Specification; }
		virtual Ref<Texture2D>					GetColorRT(int index) override;
		virtual Ref<Texture2D>					GetDepthStencilRT() override;
		virtual Ref<RenderPass>					GetRenderPass() override { return std::static_pointer_cast<RenderPass>(m_RenderPass); }

	private:
		void CreateBuffer();
	private:
		uint32_t										m_FrameBuffer = 0;
		Ref<OpenGLRenderPass>							m_RenderPass;
		uint32_t										m_MultiSampleFrameBuffer = 0;
		uint32_t										m_DepthStencilAttachment = 0;
		std::vector<uint32_t>							m_ColorAttachments;
		uint32_t										m_MultiSampleDepthStencilAttachment = 0;
		std::vector<uint32_t>							m_MultiSampleColorAttachments;
		std::vector <Ref<OpenGLTexture2D>>				m_ColorRTs;
		Ref<OpenGLTexture2D>							m_DepthStencilRT;
	};
}


