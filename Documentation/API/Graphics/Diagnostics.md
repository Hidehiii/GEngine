## Diagnostic storage-buffer readback

`StorageBuffer::ReadData(size, destination, offset = 0)` copies bytes to CPU
memory synchronously and validates bounds in Debug and Release. The buffer must
be in storage/UAV state. D3D12 and Vulkan wait for submitted queues, stage a copy,
and wait for that copy before reading; OpenGL uses its buffer-update barrier and
synchronous readback. This is for diagnostics and infrequent results, not
per-frame streaming. Call it after submitting producers, outside recording
callbacks. FrameGraphTriangle checks an actual compute result once.

D3D12 storage-buffer creation now transitions the buffer from its actual
COMMON initial state to UAV before `SetData`, `ReadData`, or a graph import
assumes ShaderWrite. No public signature changed. Debug Layer-on and Release
120-frame FrameGraphTriangle runs passed both readbacks and no longer logged
the initial-state warning. Validation-enabled private-byte growth remains a
separate diagnostic issue.

## Device-owned creation

`Graphics::GetTransferCommandBuffer()` now acquires a transfer command buffer
from the active backend. D3D12/Vulkan return native copy/transfer recording;
OpenGL returns a graphics command buffer on its single context. This is a
low-level acquisition path. `Graphics::PrepareTransferResource` is the graph's
backend-neutral handoff hook: D3D12 prepares 2D textures/arrays in COMMON
before COPY-list use, while Vulkan/OpenGL use their ordinary transfer-state
barriers. The graph transfer builder is verified for the documented
texture-to-array copy path.
The FrameGraphTriangle transfer-copy regression now passes Debug and Release
on OpenGL, D3D12, and Vulkan shared/dedicated queues; this verifies the
texture-to-array copy path, not arbitrary transfer operations.
After a D3D12 transfer copy, native texture trackers treat the resource as
COMMON before a later graphics transition or diagnostic readback; this does
not add a public copy-method argument.
Vulkan's transfer acquisition now returns a reusable wrapper for the native
transfer command pool; previously only graphics/compute wrappers were created.

`Graphics::GetRenderDevice()` now creates shaders, materials, graphics/compute
pipelines, samplers, storage buffers/images, 2D/array/cube textures and combined
texture/sampler wrappers, in addition to render passes, framebuffers and mesh
buffers. Native backend selection lives in backend implementations, not these
resources' static factory functions.

```cpp
auto& device = Graphics::GetRenderDevice();
auto shader = Shader::Create("Assets/Shaders/FrameGraphTriangle.gasset");
auto material = device.CreateMaterial(shader, "Triangle");
auto pipeline = device.CreateGraphicsPipeline(material, vertices);
```

`device.CreateShader(path)` creates an uncached backend object;
`Shader::Create(path)` retains the existing runtime-scoped cache behavior.
Similarly, `Sampler::Create` caches while `device.CreateSampler` allocates a new
sampler. Other static factories remain compatibility adapters. This does not yet
support multiple simultaneously active devices or cross-device resource mixing.

`Material::GetShader()` now returns `Ref<Shader>` by value. The previous reference
return exposed a temporary shared_ptr on OpenGL/Vulkan; callers should hold the
returned shared_ptr rather than binding a mutable reference to it.

`RenderDevice` also owns creation of render passes/framebuffers and mesh
buffers. `GraphicsQueue::Submit` accepts a completed command buffer of its
queue type; `WaitForIdle` is for shutdown or exceptional diagnostics, not a
normal per-frame dependency. `GraphicsPresent` owns swap-chain acquisition and
submission; do not submit its command buffer yourself. OpenGL, Vulkan, and
D3D12 are separate implementations of these common contracts, not three
interchangeable native-handle types.

`RenderSystem::Resize(width, height)` forwards nonzero sizes to the presenter's
`Resize` hook after updating the viewport. D3D12 rebuilds its swap-chain
targets and presenter command lists when dimensions change; the default hook
does nothing for OpenGL and Vulkan, which retain their existing resize paths.
The runtime skips zero-size events. This is a lifecycle hook, not an
application-level resize API; the user reported the interactive D3D12 check
passed, while detailed debug/memory measurements remain open.
The hook does not expose D3D12 fence handles to application code.
Example-only diagnostic: `GENGINE_EXAMPLE_SYNTHETIC_RESIZE=1` makes
FrameGraphTriangle inject 960x540 and 1280x720 resize events after frames 30
and 60 on D3D12. It does not change the native window, add a production
graphics API, or prove interactive resize behavior.
Both diagnostic D3D12 resize events completed in Debug and Release with the
same frame-119 readbacks. The real window resize/minimize check was reported
successful by the user, not reproduced by the agent.

No public graphics signature changes for D3D12 diagnostic isolation. In a
Debug D3D12 process, `GENGINE_D3D12_DEBUG_LAYER=1` opts into validation;
`GENGINE_D3D12_INFO_QUEUE_LOGGER=0` separately suppresses the polling logger
for memory comparisons without disabling the layer. Keep the logger enabled
for runs where warning/error output is required; see
[D3D12 diagnostics](../../Diagnostics/D3D12Diagnostics.md).
`GENGINE_D3D12_LIFETIME_TRACE=1` is also diagnostic-only: it records pipeline
and context-retirement counts without adding or changing a public API.
`GENGINE_D3D12_LIVE_OBJECTS=1` adds no public graphics signature. With
`GENGINE_D3D12_DEBUG_LAYER=1` in a Debug build, it requests a live-device-object
summary at context shutdown and copies available report messages into the
engine log. Normal runs keep it disabled. The context still owns its own
queues, swap chain, and heaps at this point, so compare counts across matched
runs rather than expecting a zero-object report.
The 120/60,000-frame Debug Layer-on checks produced identical shutdown
category counts; layer-off controls stayed near-flat in process private bytes.
This diagnostic is not a heap profiler and does not expose an application API
for allocation ownership.
WPR heap tracing is an external diagnostic, not a GEngine API or runtime
option; it requires a separate privileged per-process tracing configuration.
The current session could query but not enable that configuration
(`0x80070005 Access is denied`), so no heap trace or allocation-stack result
is implied by the live-object summary.
The completed, user-authorized elevated WPR capture was an external tool
workflow with no new GEngine public API. Its layer-on/off comparison found
more outstanding heap allocations
in the validation-enabled interval, including an SDK-layer stack called
through command-buffer reset. It does not change `GraphicsPresent` or
`CommandBuffer` API semantics and is not a reason to add per-frame waits.

For scheduled GPU work, `RenderGraph` imports resources or declares transient
texture-array, 2D texture, storage-buffer, and storage-image descriptions.
`RenderDevice::ReleaseTransferOwnership` and `AcquireTransferOwnership` are
graph-facing hooks for native queue-family ownership. Vulkan implements them
for opt-in exclusive `Texture2D`/`Texture2DArray` subresource ranges;
single-family and concurrent resources require no ownership barrier. Other
backends have no Vulkan-style queue families, so these hooks do no extra work.
Application code should declare graph accesses rather than call the hooks.
`GENGINE_VULKAN_EXCLUSIVE_TRANSFER_IMAGES=1` enables the opt-in image path;
`GENGINE_VULKAN_EXCLUSIVE_COMPUTE_BUFFERS=1` also opts Vulkan storage buffers
into exclusive sharing. The graph uses `ReleaseComputeOwnership` and
`AcquireComputeOwnership` for graphics/compute handoff, and
`GetQueueOwnershipKind` for preflight and command capacity. This path is
validated for the FrameGraphTriangle buffer pattern on distinct graphics and
compute families; compute images remain unsupported. The default
buffer allocation continues using concurrent sharing across distinct families.
`BuildGraphicsPass` records into a target, `BuildGraphicsCopyPass` records a
targetless graphics-queue copy, and `BuildComputePass` records dispatch.
Declare `Read`/`Write` states and ranges before `ExecuteGpu`; use versioned
resources to order producers and consumers. See the detailed
[render-graph API](../RenderGraph.md). The common device method
`CopyTexture2DToArrayLayer(command, source, destination, layer)` validates
matching size/format and the destination layer. D3D12 and Vulkan accept
graphics or transfer command buffers; OpenGL uses its graphics-context
fallback. `Texture2DArray::ReadLayerData`
is an exact-size RGBA8 synchronous diagnostic readback, not streaming I/O.
