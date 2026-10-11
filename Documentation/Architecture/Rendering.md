# Rendering and backend architecture

[Architecture overview](Architecture.md) · [Graphics API](../API/GraphicsAPI.md) ·
[Render graph API](../API/RenderGraph.md)

## Frame work and graph

`RenderSystem` owns the presentation graph and `FrameContext`. Per frame it
acquires the swap-chain image, records `LayerRender` (`Layer::OnRender`), an
optional ImGui pass, and `Present` (`Layer::OnPresent`), transitions the
presentation image to a render target and back, then submits. `RenderGraph`
expresses pass ordering, resource versions, resource states, and subresource
ranges. D3D12 records resource/UAV barriers, Vulkan records image/buffer/memory
barriers, and OpenGL emits memory barriers without tracking image layouts.

Renderer features can use a persistent GPU graph with `BuildGraphicsPass`,
`BuildGraphicsCopyPass`, `BuildComputePass`, graph-owned targets, and
`ExecuteGpu`. The graph owns command Begin/End, attachment binding, dependency
submission, and the presentation join. Versioned resources order producers,
readers, and overwrites independently of declaration order. CPU-only
`BuildPass`/`Execute` remains a compatibility path, not cross-queue GPU
synchronization. Disjoint ranges are still conservatively ordered; a dedicated
transfer-queue builder and exclusive Vulkan queue-family ownership are not
implied. See [RenderGraph.md](../API/RenderGraph.md) for the exact contract.

Transient `Texture2D`, texture-array, storage-buffer, and storage-image
descriptions are allocated at graph compilation, exposed during pass execution,
and released at reset. The graph does not yet pool or alias their memory before
resource lifetime analysis.

## Device queues and presentation

`RenderDevice` exposes portable `GraphicsQueue` instances for graphics,
compute, and transfer work. A `CommandBuffer` records with `Begin`/`End`;
submission is a separate queue operation. `Graphics::SubmitCommandBuffer`
remains a compatibility facade. `GraphicsQueue::WaitForIdle` is for shutdown
and exceptional synchronization, not routine per-frame lifetime management.

Presentation owns acquire, submit, fence, and present synchronization. On
Vulkan, inter-command dependencies live in the device submission layer;
the presenter consumes the acquire semaphore and produces the present
semaphore. Resize compares the requested viewport with swap-chain extent
before acquisition, avoiding an abandoned signaled acquire semaphore.
Vulkan allocations use concurrent sharing across distinct graphics, compute,
and transfer families; dependency semaphores provide memory ordering. D3D12
dependencies identify a producer submission generation rather than a mutable
shared fence value. OpenGL executes the logical chain on one context.

## Backend layering and shader ownership

Application and renderer code use `GEngine/include/GEngine` interfaces.
`GEngine/src/Platform` and `GEngine/include/Platform` contain OpenGL, Vulkan,
and D3D12 resource, command, context, and presentation implementations;
`Surface` contains GLFW and Win32 windows/input adapters. Native handles,
allocation policies, and synchronization remain behind those boundaries.

OpenGL backend construction selects an API without issuing GL calls. Window
creation makes the context current and loads GLAD; `GraphicsRuntime::Initialize`
then sets up debug output/default GL state. Loader failure throws before a GL
state call, including in Release. OpenGL translates the compiler's Vulkan
SPIR-V intermediate to desktop GLSL 450 using SPIRV-Cross; it does not submit
Vulkan binaries to `glShaderBinary`. Shader compile/link failures throw and
release partial programs. Both vertex-buffer constructors create a VAO and
destruction deletes it; non-indexed draws do not require an index buffer.
Complex texture/sampler combinations still need focused coverage.

`Shader` owns reflection data and compiled bytes. Reflected vertex inputs set
buffer format/offset/stride; D3D12 uses DXIL reflection and Vulkan uses SPIR-V
reflection. `Shader::Create(path)` checks a cache keyed by active API and
lexically normalized absolute path before backend construction. Relative
paths resolve against the working directory; symlinks/case variants are not
canonicalized. `GetShader(name)` is a compatibility lookup, and
`ShutdownCache` clears both caches while the context is alive. Cache lookup
is render-thread-only, runtime-scoped, and not hot reload.

## GPU resource lifetime

Vulkan replacement uses monotonic submission serials. Graphics, compute, and
transfer submissions have completion fences; presentation has an additional
completion marker. A contiguous completion watermark retires resources,
descriptor sets, shader modules/layouts, and render passes. The descriptor
pool is context-owned and destroyed only after device wait and retirement
flush. Short-lived uploads remain synchronous. Callers retain owners until
recorded commands are submitted; unsubmitted commands are not tracked.

D3D12 has separate mechanisms: context submission tracking retires replaced
native buffers/pipelines across queues, while each command buffer retains
recording owners and uses its own monotonic fence value before allocator reset.
Presentation reuses the same reset path and has one command buffer/fence value
per swap-chain frame slot. This protects whole-object replacement, not in-place
mutation of a retained resource. Native command-list users still retain their
resources. The D3D12 debug layer is opt-in with
`GENGINE_D3D12_DEBUG_LAYER=1`; see
[D3D12 diagnostics](../Diagnostics/D3D12Diagnostics.md).

## Extension points

- Put cross-backend concepts in `GEngine/include/GEngine` and portable
  orchestration in `GEngine/src/GEngine`.
- Add backend implementations only for native behavior; do not expose native
  handles through application/renderer interfaces.
- Declare resource access/state through `RenderGraph` rather than a layer's
  backend-specific barriers.
- Exercise substantial rendering work in a focused `Example` before editor
  integration.
