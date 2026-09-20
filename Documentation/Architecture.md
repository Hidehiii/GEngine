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

During shutdown, the order is deliberately reversed for GPU safety: the render
system waits for submitted work, layers and ImGui release their GPU resources,
shared resource caches are cleared, and only then does the window destroy the
native graphics context/device. This order is essential for Vulkan because an
object cannot be destroyed while a submitted command buffer still references it.

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

The render graph expresses pass ordering and resource-state transitions. D3D12
records resource/UAV barriers, Vulkan records image/buffer/memory barriers, and
OpenGL emits memory barriers for visibility rather than tracking image layouts.

Renderer features can use a persistent GPU graph with `BuildGraphicsPass`,
`BuildComputePass`, graph-owned targets and `ExecuteGpu`. The graph owns
Begin/End, attachment binding, dependency submission and the presentation join.
Versioned resources order producers, readers and overwrites independently of
declaration order. CPU-only `BuildPass`/`Execute` remains a compatibility path;
it is not a substitute for cross-queue GPU synchronization.

Vulkan engine allocations use concurrent sharing across distinct graphics,
compute and transfer families; dependency semaphores provide memory ordering
and are retired after their consumer completes. D3D12 dependencies identify a
specific producer command-buffer submission generation, not a mutable shared
fence value. OpenGL executes the same logical chain on its single context.
See [RenderGraph.md](RenderGraph.md) for the supported whole-resource and
storage-state contract and its current limitations.

## Device queues and command submission

`RenderDevice` exposes portable `GraphicsQueue` instances for graphics,
compute, and transfer command buffers. A command buffer records work with
`Begin`/`End`; queue submission is a separate operation:

```cpp
auto commandBuffer = Graphics::GetGraphicsCommandBuffer();
// Record work, then call commandBuffer->End().

auto& queue = Graphics::GetRenderDevice().GetQueue(COMMAND_BUFFER_TYPE_GRAPHICS);
queue.Submit(commandBuffer);
```

The old `Graphics::SubmitCommandBuffer` function remains as a migration facade.
New renderer code should obtain a queue from `RenderDevice`, which keeps queue
selection explicit and does not expose Vulkan, D3D12, or OpenGL native types.

`GraphicsQueue::WaitForIdle` is intended for deterministic shutdown and
exceptional synchronization, not normal per-frame rendering. Presentation owns
its acquire, submit, fence, and present synchronization internally. On Vulkan,
inter-command dependencies live in the device submission layer rather than in
the command-buffer wrapper; the swapchain presenter directly consumes the
acquire semaphore and produces the present semaphore for its frame.

When a Vulkan window is resized, the presenter compares the requested viewport
with the current swapchain extent before acquiring an image. If recreation is
needed, it recreates the swapchain without acquiring first. This prevents an
acquire semaphore from being signaled and then abandoned during recreation.

## Vulkan replacement lifetime

Vulkan replacement uses a monotonically increasing submission serial. Graphics,
compute, and transfer queue submissions receive dedicated completion fences;
presentation receives an additional completion marker after its existing submit.
A contiguous completion watermark retires resources without requiring reusable
frame fences to be signaled simultaneously. Collection runs at submission and
frame acquisition; device idle flushes all remaining entries. These operations
run on the render thread. Callers must retain resource owners until recorded
commands have been submitted; unsubmitted commands are not tracked.

The same retirement queue now covers vertex, index, uniform, and storage
buffers, sampled/storage images, samplers, and framebuffers. Short-lived upload
commands remain synchronous by design; frame-recorded resources do not stall
the whole device when their C++ owner is released.

Material descriptor sets and shader-owned modules, pipeline layouts, and
descriptor-set layouts use the same submission watermark. Retirement callbacks
capture native handles by value, not the destroyed C++ owner. The shared
descriptor pool is context-owned and is destroyed only after shutdown waits for
the device and flushes retired sets. It is not a replaceable per-material pool.
Shader cache ownership still controls when shader destruction begins.

### Shader factory caching

`Shader::Create(path)` first checks an in-memory cache keyed by the active API
and lexically normalized absolute path, before constructing any backend shader.
Relative paths are resolved against the current working directory; `.` and `..`
segments are normalized. Symlink aliases and case variations are not canonicalized.
`GetShader(name)` remains the compatibility lookup: distinct source files with
the same declared name still resolve to the first loaded shader. Both caches are
cleared by `ShutdownCache` while the graphics context is alive. Cache operations
are render-thread-only and scoped to one active runtime, not a multi-device cache.
Repeated creation is not hot reload; edits do not invalidate the cached object.

## Frame-graph transient resources

`RenderGraph` supports portable descriptions for transient `Texture2D`, storage
buffer, and storage-image resources. A transient resource is declared by name,
created only when the graph compiles, exposed through a typed getter during pass
execution, and released when the graph resets. This is the allocation baseline
for future frame-local pooling and aliasing; it deliberately does not reuse
memory before resource lifetime analysis exists.

The same retirement queue now covers vertex, index, uniform, and storage
buffers, sampled/storage images, samplers, and framebuffers. Short-lived upload
commands remain synchronous by design; frame-recorded resources do not stall
the whole device when their C++ owner is released.

## Renderer layering

OpenGL backend construction only selects the API; it must not issue GL calls.
Window creation makes the context current and loads GLAD. The subsequent
`GraphicsRuntime::Initialize` calls `OpenGLGraphicsAPI::Initialize` once to set
up debug output and default GL state.
Loader failure throws before any GL state call, including in Release builds.

OpenGL translates the compiler's Vulkan SPIR-V intermediate to desktop GLSL 450
using SPIRV-Cross before driver compilation. It does not submit Vulkan binaries
to `glShaderBinary`. Stage compilation and linking failures throw with a log and
release partially created programs. Complex texture/sampler binding combinations
still require dedicated regression coverage; the current test is non-textured.
Both vertex-buffer constructors create a VAO and destruction deletes it.
Attribute component counts are distinct from byte strides, and non-indexed
draws do not require an index-buffer object.

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

Context-owned submission tracking also retires replaced native vertex/index
buffers and graphics pipeline states across all queues. Its completion watermark
is separate from the command-buffer completion used for allocator reuse below.
Both mechanisms remain after cross-computer integration.

D3D12 command buffers retain draw/dispatch pipeline owners and explicitly
transitioned resources until their recording is reset after GPU completion.
Each command buffer owns a fence/event with a monotonic submission value,
signaled by the submitting queue. Allocator reuse waits for that value and then
releases retained owners; destruction also waits. Presentation uses the same
reset path as offscreen commands. This protects whole-object pipeline/material/
buffer replacement without a device-wide per-frame wait. It does not make
in-place mutation of a retained pipeline or material safe while in flight.
Native command-list access still requires the caller to retain referenced
resources; the synchronous single-time helpers keep their existing wait path.

There is one presentation command buffer and fence value per swap-chain frame slot. Before reusing a slot, `D3D12GraphicsPresent` waits for that slot's fence and resets both its command allocator and command list. This is required: resetting only the list makes allocator memory grow for every submitted frame.

The default debug build does not enable the D3D12 debug layer automatically, because validation output can be expensive in a continuous render loop. Set `GENGINE_D3D12_DEBUG_LAYER=1` for a diagnostic run; errors and warnings are drained from a bounded info queue.

## Extension points

- Put new cross-backend concepts in `GEngine/include/GEngine` and their backend-neutral orchestration in `GEngine/src/GEngine`.
- Add a backend implementation only where the API requires native behavior; keep backend types out of client-facing headers.
- Add externally visible resource access and state requirements through `RenderGraph`, rather than hand-writing D3D12 barriers in a layer.
- Prefer a focused example project for a new rendering feature before integrating it into the editor.
