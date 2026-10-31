## Graph-owned recording and attachments

```cpp
RenderGraph::AttachmentSpecification attachment{};
attachment.ColorFormats = { FRAME_BUFFER_TEXTURE_FORMAT_RGBA8 };
attachment.DepthStencilFormat = FRAME_BUFFER_TEXTURE_FORMAT_DEPTH24_STENCIL8;
auto target = graph.CreateRenderTarget("Scene", attachment, 512, 512);
auto draw = graph.BuildGraphicsPass("SceneDraw", target,
    [&](const Ref<CommandBuffer>& command) { command->Render(pipeline, 0); });
auto compute = graph.BuildComputePass("Update", [&](const Ref<CommandBuffer>& command) {
    command->Compute(computePipeline, 0, 1, 1, 1);
});
compute.DependsOn(draw.GetHandle());
// All graph work joins this still-unsubmitted presentation command buffer.
graph.ExecuteGpu(GraphicsPresent::GetCommandBuffer());
```

The graph creates targets after dependency validation, begins command recording,
emits resource barriers outside render passes, binds attachments, invokes the
record callback, ends recording and submits to the declared queue. Callbacks
must not begin/end or submit their command buffer. Color attachments are
automatically declared as writes and end in ShaderRead, matching the native
framebuffer contract. `GetColorAttachment` exposes their graph resource handle;
`GetFrameBuffer` is available after successful compilation. Depth remains owned
by the target. The current path uses framebuffer clear/store operations, not
arbitrary per-pass load/store overrides.

`AttachmentSpecification` is the graph-facing attachment contract. It describes
color formats, depth format, sample count, load/store operations and optional
subpass attachment indices without exposing Vulkan stage/access masks. A
subpass-enabled descriptor is translated to the backend render-pass contract
only when `GraphicsCapabilities::Subpasses` is available; otherwise compilation
throws before target allocation. `CreateRenderTarget(RenderPassSpecification)`
remains a compatibility adapter for existing callers, but new graph code should
use the portable descriptor.

Keep a compiled GPU graph alive across frames to reuse its target allocation;
Reset invalidates handles and releases its owned targets. Rebuild it for a new
size. Pass callbacks can reference current replacement pipelines through the
owning layer. Command buffers are acquired afresh per execution. The configured
command-buffer count bounds passes per queue (one graphics slot is reserved).

Every dependency is registered before producer submission; completion joins
all graph submissions. D3D12 waits for a producer command buffer's monotonic
submission generation. Vulkan uses independently consumable binary semaphores
with ALL_COMMANDS waits, retired after consumer completion. Vulkan engine
buffers/images use concurrent sharing by default when graphics/compute/transfer
families differ. Opt-in exclusive 2D/array images use the release/acquire path
described above; other allocations remain concurrent. OpenGL
executes on its single context and uses memory barriers for visibility.

For D3D12 UAV resources reused across executions, backend submission also waits
on the resource's last actual queue use. A compute-first graph therefore waits
for the preceding frame's final graphics use without a CPU device-wide idle.
This uses the existing graph API; it does not enable independent subresource
scheduling or replace the graph's declared dependencies.

Current GPU compute declarations accept storage/UAV (`ShaderWrite`) state for
both read and write intent. ShaderWrite names the native state, not exclusively
the access direction. Other compute states are explicitly rejected rather than
emitting graphics-only stages on a compute queue. Access declarations identify
graphics, compute, or transfer usage; omitted stage declarations remain
conservative (`All`). Shader states accept graphics or compute, copy states
require transfer, and render-target, depth-write, and presentation states
require graphics. A graphics pass may also use the transfer stage for a copy;
compute and transfer stages are rejected on incompatible pass queues during
compilation. Write-only states cannot be declared as reads and read-only states
cannot be declared as writes. Vulkan narrows its stage/access masks from that
validated intent, while D3D12 legacy barriers and OpenGL memory barriers
preserve the same ordering conservatively. Texture subresource dependency
analysis is capability-gated; pooling remains unavailable. Remaining
graph-access work is tracked in
[RenderingArchitecture.md](../../../TODO/RenderingArchitecture.md).

`SubresourceRange` is available on the legacy `Read` and `Write` declarations
for validation and range-aware scheduling.
The default denotes the whole resource.
Empty and overflowing ranges are rejected when the access is declared. During
compilation, `All` counts are resolved against the imported or transient
resource's mip/layer metadata before transient allocation. Out-of-bounds,
zero-count, and non-whole ranges on resources without subresource metadata
(including buffers and logical imports) are rejected before allocation or
recording. Imported textures expose their actual mip count; texture arrays
expose their layer count; 2D storage images currently expose one mip and one
layer. `ExecuteGpu` asks the active backend to validate every compiled range
before acquiring or recording command buffers. D3D12 tracks native state per
texture mip/array layer; Vulkan tracks image layout per mip/layer and emits one
`VkImageMemoryBarrier` per selected cell. OpenGL accepts valid texture ranges
but uses its resource-global `glMemoryBarrier`, so those declarations never
enable independent layer scheduling. Buffers remain whole-resource, and Vulkan
image types without range tracking reject non-whole ranges. Ranges are never
silently widened by a backend that cannot preserve their state.
For Vulkan cross-queue edges, submission semaphores carry the prior queue's
memory dependency. The barrier recorded on the receiving queue uses only stages
supported there; an unsupported prior-queue source stage becomes a top-of-pipe
scope with no source access mask. Unsupported destination stages are rejected.

Dependency analysis compares declared ranges. When
`GraphicsCapabilities::IndependentSubresourceScheduling` is true and the
resource has mip/layer metadata, D3D12 and Vulkan omit inferred dependencies
between disjoint texture ranges. OpenGL and resources without subresource
metadata remain conservatively ordered. Compiled transition state is split by touched
mip/layer ranges, so an access to one mip does not consume the state of
another. `SetTransitionUsageCallback` receives the transition range and
`RenderSystem` forwards it to the range-aware `Graphics::TransitionResource`
overload. Unsupported backend/resource combinations fail in the graph's
pre-recording validation pass.

`Execute` compiles an uncompiled graph in both Debug and Release. A dependency
cycle throws `std::runtime_error` before pass execution or transient allocation.
