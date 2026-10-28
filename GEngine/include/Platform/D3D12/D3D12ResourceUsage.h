#pragma once
#include "GEngine/Graphics/GraphicsCommon.h"
#include <d3d12.h>
#include <wrl.h>

namespace GEngine
{
    // Last submitted access, never a mutable command-buffer or fence-value alias.
    struct D3D12ResourceUsage
    {
        Microsoft::WRL::ComPtr<ID3D12Fence> Fence;
        uint64_t Value = 0;
        CommandBufferType Queue = COMMAND_BUFFER_TYPE_NONE;
    };
}
