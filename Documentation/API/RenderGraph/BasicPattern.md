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
`BuildTransferCopyPass(name, record)` declares a transfer-stage pass. Declare
its reads as `CopySource` and writes as `CopyDestination`, both with
`GraphicsPipelineStage::Transfer`. D3D12/Vulkan record the copy on their
copy/transfer queues after a graphics-side preparation submission; OpenGL
executes on its single graphics context. Completion joins later consumers and
presentation. Accesses with unchanged copy state are prepared as well.
D3D12 currently supports 2D textures and texture arrays on this path and
rejects other resource types before copy submission. Vulkan resources use
concurrent queue-family sharing by default; opt-in exclusive 2D/array images
use ownership barriers on this path. `BuildGraphicsCopyPass` remains available.
`GENGINE_VULKAN_EXCLUSIVE_TRANSFER_IMAGES=1` opts Vulkan `Texture2D` and
`Texture2DArray` allocations into exclusive sharing for transfer-copy testing.
`ExecuteGpu` records matching release/acquire barriers for their declared
subresource ranges around the transfer queue submission. Debug/Release
dedicated-family example runs passed both pixel readbacks with no captured
validation issues; concurrent sharing is still the default. That image-only
milestone did not cover exclusive buffers or compute handoffs.
An opt-in 10,000-frame Debug check and a 32-second memory sample also passed;
neither proves arbitrary-resource or indefinite lifetime correctness.
`GENGINE_VULKAN_EXCLUSIVE_COMPUTE_BUFFERS=1` opts Vulkan storage buffers into
exclusive sharing. `ExecuteGpu` adds graphics preparation before an accessing
compute pass, performs graphics/compute release-acquire, and acquires back on
later graphics access. Unsupported exclusive image/compute or buffer/transfer
combinations are rejected before submission. The example path is runtime-
validated; a shared graphics/compute family needs no ownership preparation.
FrameGraphTriangle has focused startup regressions for both unsupported
combinations. With dedicated Vulkan queues and both opt-ins, it logs that
exclusive transfer-image compute access and exclusive compute-buffer transfer
access are rejected before submission, while the normal graph still passes all
readbacks and shuts down normally.
No new application-facing pass builder is needed. Exclusive
compute image ownership remains outside this step and is tracked in
[RenderingArchitecture.md](../../../TODO/RenderingArchitecture.md).
The example now includes a compute-first buffer with a subsequent graphics
draw and readback, in addition to its graphics-first storage buffer. Both
readbacks passed in Debug/Release on Vulkan dedicated queues with the opt-in.
OpenGL, D3D12, and Vulkan shared/default runs passed as regression controls.
This is bounded example evidence, not arbitrary buffer or external-queue proof.
FrameGraphTriangle uses the transfer builder for a real array-layer pixel
copy, retains invalid layer/size/format checks, and passed 120-frame Debug
and Release runs on OpenGL, D3D12, Vulkan shared, and Vulkan dedicated queues.
On 2026-10-31, the current backend-module sources were rebuilt and rechecked:
the same Debug/Release matrix plus Vulkan dedicated queues passed all three
GPU readbacks and normal shutdown. D3D12 used its debug layer and reported no
issues; the dedicated Vulkan run selected graphics=0, compute=2, transfer=4.
Debug D3D12 and Vulkan dedicated additionally completed 10,000-frame runs;
the existing array-layer readback still passed, with no captured validation
issues. These checks do not cover arbitrary transfer resource types.

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
