# Architecture

## Top-level layout

| Location | Responsibility |
| --- | --- |
| `GEngine/include` | Public engine headers consumed by applications and examples. |
| `GEngine/src/GEngine` | Backend-neutral runtime, graphics abstractions, renderer, scene, components, tools, and physics integration. |
| `GEngine/src/Platform` | OpenGL, Vulkan, and D3D12 implementations of graphics abstractions. |
| `GEngine/src/Surface` | GLFW and Win32 window/input implementations. |
| `Example` | Small runnable programs that exercise engine features. |
| `GEngine-Editor` | Editor application. |
| `GEngine/vendor` | Third-party libraries. |

## Ownership and startup

`Application` is the application-facing facade. It owns one `EngineRuntime`, which owns the window, layer stack, render system, configuration and optional services.

```text
Client CreateApplication
        |
   Application
        |
   EngineRuntime
   |     |       |
Window  Layers  RenderSystem
                   |
             Graphics / backend
```

During startup, `EngineRuntime` reads `Config.ini`, configures `Graphics`, creates the native window and graphics context, initializes `RenderSystem`, then attaches application layers. `Graphics` selects the render-device implementation from `Graphics_API`.

## Per-frame flow

`EngineRuntime::RunFrame` updates the layers and delegates rendering to `RenderSystem`.

```text
OnUpdate
  -> RenderSystem::RenderFrame
     -> acquire swap-chain image and select its frame slot
     -> record LayerRender pass (Layer::OnRender)
     -> optionally record ImGui pass
     -> transition the presentation image to RenderTarget
     -> begin presentation target, clear it, call Layer::OnPresent
     -> transition it back to Present and submit
  -> OnLateUpdate / OnEndFrame
  -> window event processing
```

The render graph expresses pass ordering and resource-state transitions. On D3D12, its transition callback records the matching `ID3D12GraphicsCommandList::ResourceBarrier` calls. Backends that do not need explicit resource-state barriers simply ignore that callback.

## Renderer layering

The renderer separates portable intent from backend implementation:

| Portable API | D3D12 implementation example | Purpose |
| --- | --- | --- |
| `GraphicsPresent` | `D3D12GraphicsPresent` | Acquire, record, submit and present a frame. |
| `GraphicsPipeline` | `D3D12GraphicsPipeline` | Build/bind pipeline state and issue draw calls. |
| `VertexBuffer` / `IndexBuffer` | `D3D12VertexBuffer` / `D3D12IndexBuffer` | Store mesh data and bind the input assembler. |
| `Shader` / `Material` | `D3D12Shader` / `D3D12Material` | Compile, reflect, bind resources and create root signatures. |
| `Texture`, `FrameBuffer`, `RenderPass` | D3D12 equivalents | Manage images, render targets and depth buffers. |

`Shader` owns reflection data. The reflected vertex inputs determine vertex-buffer format, offset and stride. D3D12 uses DXIL reflection; Vulkan uses SPIR-V reflection. A shader's compiled bytes are owned by the shader object, so D3D12 pipeline bytecode pointers remain valid for the pipeline lifetime.

## D3D12 frame lifetime

There is one presentation command buffer and fence value per swap-chain frame slot. Before reusing a slot, `D3D12GraphicsPresent` waits for that slot's fence and resets both its command allocator and command list. This is required: resetting only the list makes allocator memory grow for every submitted frame.

The default debug build does not enable the D3D12 debug layer automatically, because validation output can be expensive in a continuous render loop. Set `GENGINE_D3D12_DEBUG_LAYER=1` for a diagnostic run; errors and warnings are drained from a bounded info queue.

## Extension points

- Put new cross-backend concepts in `GEngine/include/GEngine` and their backend-neutral orchestration in `GEngine/src/GEngine`.
- Add a backend implementation only where the API requires native behavior; keep backend types out of client-facing headers.
- Add externally visible resource access and state requirements through `RenderGraph`, rather than hand-writing D3D12 barriers in a layer.
- Prefer a focused example project for a new rendering feature before integrating it into the editor.
