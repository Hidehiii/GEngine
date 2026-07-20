#pragma once
#include "GEngine/Graphics/Material.h"
#include "Platform/D3D12/D3D12Shader.h"
#include "Platform/D3D12/D3D12UniformBuffer.h"
#include "Platform/D3D12/D3D12DescriptorHeap.h"
#include <windows.h>
#include <wrl.h>
#include <dxgi1_6.h>
#include <directx/d3dx12.h>

namespace GEngine
{
	class GENGINE_API D3D12Material : public Material
	{
	public:
		D3D12Material(const Ref<Shader>& shader, const std::string& name = "");

		virtual Ref<Shader>&		GetShader() override { return m_ShaderBase; }
		virtual Buffer				SetUniformBuffer(const uint32_t& pass, const uint32_t& bindPoint, const Buffer& buffer, const Ref<UniformBuffer>& buf) override;
		virtual void				Update(CommandBuffer* cmdBuffer, const uint32_t& pass) override;
		virtual void				ResourceUpdateNotify() override { CreateDescriptorHeap(); }

		const std::vector<D3D12DescriptorHeap::D3D12DescriptorAllocationInfo>&	GetCbvSrvUavHeaps() const { return m_CbvSrvUavHeaps; }
		const std::vector<std::unordered_map<uint32_t, Ref<D3D12UniformBuffer>>>&	GetConstantBuffers() const { return m_ConstantBuffers; }

		bool operator==(const D3D12Material& other) const
		{
			return m_Shader == other.m_Shader && m_ConstantBuffers == other.m_ConstantBuffers;
		}
	private:
		void CreateDescriptorHeap();
	private:
		Ref<Shader>																m_ShaderBase;
		Ref<D3D12Shader>													m_Shader;
		std::vector<std::unordered_map<uint32_t, Ref<D3D12UniformBuffer>>>	m_ConstantBuffers; // pass { bind point : buffer }
		std::vector<D3D12DescriptorHeap::D3D12DescriptorAllocationInfo>		m_CbvSrvUavHeaps;							
	};
}


