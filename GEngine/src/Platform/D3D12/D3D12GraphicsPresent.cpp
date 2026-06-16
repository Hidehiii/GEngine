#include "GEpch.h"
#include "D3D12GraphicsPresent.h"
#include "Platform/D3D12/D3D12Context.h"

namespace GEngine
{
	D3D12GraphicsPresent::D3D12GraphicsPresent()
	{
	}

	bool D3D12GraphicsPresent::AquireImage()
	{
		return false;
	}

	void D3D12GraphicsPresent::Begin()
	{
	}

	void D3D12GraphicsPresent::End()
	{
		if (D3D12Context::Get()->IsVSync())
		{
			UINT syncInterval = 1; // Enable VSync
			UINT presentFlags = 0;
			D3D12Context::Get()->GetSwapChain()->Present(syncInterval, presentFlags);
		}
		else
		{
			UINT syncInterval = 0; // Disable VSync
			UINT presentFlags = DXGI_PRESENT_ALLOW_TEARING;
			D3D12Context::Get()->GetSwapChain()->Present(syncInterval, presentFlags);
		}
	}

}


