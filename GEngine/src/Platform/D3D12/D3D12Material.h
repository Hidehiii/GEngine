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
	private:
		void CreateDescriptorHeap();
	private:
		Ref<D3D12Shader>													m_Shader;
		std::vector<std::unordered_map<uint32_t, Ref<D3D12UniformBuffer>>>	m_ConstantBuffers; // pass { bind point : buffer }
		std::vector<D3D12DescriptorHeap::D3D12DescriptorAllocationInfo>		m_CbvSrvUavHeaps;							
	};
}


