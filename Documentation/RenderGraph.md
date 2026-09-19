# Render graph

`RenderGraph` is the backend-neutral scheduler for a frame. It owns pass dependencies, per-resource access declarations and the resulting state transitions. It does not own application resources; imported resources remain owned by their texture, buffer or presentation system.

## Basic pattern

### Explicit resource versions

Use versions when dependencies must be independent of pass creation order:

```cpp
auto reader = graph.BuildPass("Reader", [](FrameContext&) { /* Record reads. */ });
auto writer = graph.BuildPass("Writer", [](FrameContext&) { /* Record writes. */ });
auto initial = graph.GetVersion(graph.ImportResource("Data"));
auto produced = writer.Write(initial, RenderGraph::ResourceState::ShaderWrite);
reader.Read(produced);
```

Each write extends the latest version. Reads depend on that version's producer;
the next writer also waits for readers of the previous version because versions
currently share physical storage. Branching writes and mixing versioned/legacy
accesses on one resource are rejected. Reset invalidates previous versions.
An initial transient version cannot be read before a producing write.
`BuildPass` remains a CPU declaration helper. Use the GPU recording builders
below when the graph should also own command recording and queue submission.

Repeated `ShaderWrite` states generate a memory dependency even without a state
change. D3D12 emits a UAV barrier, Vulkan a shader-memory barrier, and OpenGL
uses its existing memory-barrier mapping. This is whole-resource synchronization;
it does not provide subresource-level tracking.

```cpp
RenderGraph graph;

const auto geometry = graph.AddPass("Geometry", [](FrameContext& frame)
{
    // Record geometry work.
});

const auto postProcess = graph.AddPass("PostProcess", [](FrameContext& frame)
{
    // Record post-processing work.
});

graph.AddDependency(postProcess, geometry);
graph.Execute(frameContext);
```

`AddDependency(after, before)` orders CPU callbacks with `Execute`, or GPU
submissions with `ExecuteGpu`. Do not mix CPU and GPU passes in a GPU graph.

## Graph-owned recording and attachments

```cpp
RenderPassSpecification spec{};
spec.RenderTargets = { FRAME_BUFFER_TEXTURE_FORMAT_RGBA8 };
spec.DepthStencil = FRAME_BUFFER_TEXTURE_FORMAT_DEPTH24_STENCIL8;
auto target = graph.CreateRenderTarget("Scene", spec, 512, 512);
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
arbitrary per-pass load/store overrides or subpasses.

Keep a compiled GPU graph alive across frames to reuse its target allocation;
Reset invalidates handles and releases its owned targets. Rebuild it for a new
size. Pass callbacks can reference current replacement pipelines through the
owning layer. Command buffers are acquired afresh per execution. The configured
command-buffer count bounds passes per queue (one graphics slot is reserved).

Every dependency is registered before producer submission; completion joins
all graph submissions. D3D12 waits for a producer command buffer's monotonic
submission generation. Vulkan uses independently consumable binary semaphores
with ALL_COMMANDS waits, retired after consumer completion. Vulkan engine
buffers/images use concurrent sharing when graphics/compute/transfer families
differ, so those allocations need no exclusive ownership transfer. OpenGL
executes on its single context and uses memory barriers for visibility.

Current GPU compute declarations accept storage/UAV (`ShaderWrite`) state for
both read and write intent. ShaderWrite names the native state, not exclusively
the access direction. Other compute states are explicitly rejected rather than
emitting graphics-only stages on a compute queue. Barriers cover whole resources;
fine-grained stage masks, subresource scheduling and dedicated transfer passes
are not exposed by these builders yet.

`Execute` compiles an uncompiled graph in both Debug and Release. A dependency
cycle throws `std::runtime_error` before pass execution or transient allocation.

## Resources and states

Import an external resource and declare every pass access:

```cpp
const auto color = graph.ImportTexture(
    "Color",
    colorTexture,
    RenderGraph::ResourceState::ShaderRead);

graph.Write(geometry, color, RenderGraph::ResourceState::RenderTarget);
graph.Read(postProcess, color, RenderGraph::ResourceState::ShaderRead);
```

The graph adds ordering dependencies for conflicting accesses and inserts a transition whenever a resource's requested state differs from its previous state.

## Transient resources

Frame-local working resources can be declared without creating a backend object
in feature code. The graph creates them when it compiles, and their lifetime
ends when `Reset()` is called:

```cpp
const auto lighting = graph.CreateTransientTexture2D(
    "Lighting",
    { viewportWidth, viewportHeight, RENDER_IMAGE_2D_FORMAT_RGBA8_UNORM });

graph.Write(uploadPass, lighting, RenderGraph::ResourceState::CopyDestination);
graph.Read(postProcess, lighting, RenderGraph::ResourceState::ShaderRead);

// Inside a pass callback after compilation:
auto texture = graph.GetTexture2D(lighting);
```

This texture path supports sampling and copies, not attachment rendering.
The upload pass must actually initialize the texture contents. Use an imported
framebuffer attachment for render-target work until usage-aware creation exists.
Transient resources start Undefined; compilation rejects unsupported states,
read-only states declared as writes, and reads before a declared first write.
Validation happens before transient GPU allocation.

For unversioned resources, pass creation order defines conflicting access order.
`Read`/`Write` declarations may be added in any order; inferred edges are rebuilt
on every compile separately from explicit dependencies. Contradictory explicit
ordering is a dependency cycle. This is CPU scheduling, not GPU queue completion.

`CreateTransientStorageBuffer` and `CreateTransientStorageImage` follow the
same pattern. Transient resources are not pooled or aliased yet, so a feature
must not retain them after the graph reset. Imported resources remain the right
choice for application-owned or cross-frame assets.

After `Compile`, `GetResourceLifetime` reports each resource's first pass,
last pass, and final declared state. The graph uses these values as its stable
allocation-lifetime record. Versioned read/write handles are available; transient
memory reuse is not enabled.

Available states are:

| State | Typical usage |
| --- | --- |
| `RenderTarget` | Color attachment output. |
| `DepthWrite` | Depth/stencil attachment writing. |
| `ShaderRead` | Sampling or read-only shader access. |
| `ShaderWrite` | UAV/storage writes. |
| `CopySource`, `CopyDestination` | Transfer work. |
| `Present` | Swap-chain presentation. |

On D3D12, the renderer converts these states to `D3D12_RESOURCE_STATES` and records resource barriers before the pass callback. Other backends can use the same declarations for synchronization or ignore explicit barriers where their model differs.

## Presentation integration

`RenderSystem` imports the selected swap-chain image as `PresentationBackBuffer` for every frame. It records a `Present -> RenderTarget` transition before layer presentation and a final `RenderTarget -> Present` transition before submission.

Layers normally should not manipulate that swap-chain state directly. Draw through `GraphicsPresent::Render` from `Layer::OnPresent`; the renderer owns target binding, clearing, transitions, command-list close and presentation.

For Vulkan, the legacy swap-chain render pass currently owns the native
`Present <-> ColorAttachment` image-layout transitions. The graph still tracks
the presentation resource for ordering, but does not emit a duplicate native
barrier for that image. This is an implementation boundary; layer code remains
backend-neutral.

## Rules

- Declare each resource read/write access in the graph instead of adding backend-only state transitions inside a layer.
- Make write-after-read and write-after-write operations explicit through `Read`/`Write`; the graph derives ordering dependencies from them.
- Reset the graph every frame before creating that frame's passes.
- Keep pass callbacks focused on recording commands. Long-lived ownership
  belongs outside the graph; use transient descriptors only for frame-local
  working resources.
