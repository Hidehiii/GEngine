# Features and samples

## Graphics backends

| Backend | Role in the codebase | Notes |
| --- | --- | --- |
| OpenGL | Established compatibility backend | Uses GLFW OpenGL presentation. |
| Vulkan | Explicit graphics backend | Uses SPIR-V compilation/reflection and Vulkan presentation. |
| Direct3D 12 | Explicit Windows backend | Uses DXIL, reflection-driven input layouts, root signatures, descriptor heaps, command lists and swap-chain presentation. |

Backend selection is read from `Config.ini` through `GraphicsAPI`. The Triangle sample uses `GraphicsAPI: 3` for D3D12.

## Engine subsystems

- **Application/runtime:** application loop, layers, window events, time, input, configuration and logging.
- **Rendering:** shaders, materials, pipeline state, vertex/index/uniform buffers, textures, samplers, framebuffers, render passes, portable graphics/compute/transfer queues, and render graph scheduling.
- **Compute:** compute pipelines, storage buffers and storage images.
- **Scene and components:** game objects, scenes, transform and renderer components, cameras and serialization. Component renderer draw paths are currently incomplete.
- **Physics:** Box2D-backed 2D scene simulation and PhysX-backed 3D world/body wrappers, built as separate native service modules. The scene does not currently step the 3D world.
- **Tools/assets:** YAML `.gasset` envelopes, `.gscene` files, model importing, string/file helpers, serialization, shader compilation and asset meshes.
- **Editor:** a separate editor application built on the core engine.

## Example projects

| Project | Demonstrates |
| --- | --- |
| `Example/Triangle` | Minimal shader, vertex buffer, graphics pipeline and presentation draw; configured for D3D12. |
| `Example/FrameGraphTriangle` | Cross-API queue submission, presentation, render-graph, and shutdown smoke test; use it on OpenGL, Vulkan, or D3D12. |
| `Example/RenderPipeline` | Composable render pipeline framework with compute, offscreen graphics, and sampled presentation. |
| `Example/GameWorld` | C#/C++ scene-script lifecycle, failure handling and physics backend smoke checks. |
| `Example/ShaderCompile` | Shader compilation and shader asset workflow. |
| `Example/DeferredRender` | Deferred-rendering path. |
| `Example/GpuDriven` | GPU-driven rendering work. |
| `Example/InputSystem` | Input events and queries. |
| `Sandbox` | General engine experimentation. |

## Configuration

Each application can provide `Config.ini`. Important entries are:

| Key | Meaning |
| --- | --- |
| `GraphicsAPI` | Rendering backend selection. |
| `FramesInFlight` | Swap-chain/frame-resource count. |
| `VSync` | Enables synchronized presentation when nonzero. |
| `CommandBufferCount` | Pool size for general command buffers. |
| `ShaderCacheDirectory` | Relative directory for compiled shader cache data. |
| `WindowManagerAPI` | Selects GLFW or Win32 windowing. |

For D3D12 diagnostics, run a debug executable with `GENGINE_D3D12_DEBUG_LAYER=1`. This enables the D3D12 debug layer and the engine's bounded warning/error logger.
