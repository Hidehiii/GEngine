# Render pipeline framework

Lifecycle follow-up (2026-10-31) centralizes pass configuration
and freezes the graph between explicit resets. Rebuild and dimension-change
verification is tracked in [the pipeline TODO](../../../TODO/Rendering/RenderPipeline.md).
Native submission/resource retention remains backend-owned; this work adds no
per-frame device waits or pooling.

[Rendering architecture](../Rendering.md) · [Render pipeline API](../../API/RenderPipeline.md)

`RenderPipeline` is the backend-neutral composable pipeline layer above
`RenderGraph`. The old two-method `Graphics/RenderPipeline` contract is removed.
The new public header is `GEngine/Renderer/RenderPipeline.h`.

A pipeline owns one persistent `RenderGraph`, an ordered list of
`RenderPipelinePass` objects, and an explicit configure/execute/present flow:

1. A derived pipeline overrides `Configure(RenderPipelineBuilder&)`.
2. It creates targets/resources and adds ordered pass objects.
3. The framework configures all registered passes in insertion order, once per
   graph generation. Each pass builds graph passes and declares resource versions.
4. `Execute(completion, cameras)` calls per-frame pass hooks and then executes
   the graph through `RenderGraph::ExecuteGpu`.
5. `Present(command, cameras)` calls pass presentation hooks. Presentation
   remains backend-neutral through `GraphicsPresent`.

Configuration, execution and presentation are guarded phases. A configured
pipeline rejects pass-list mutation; rebuilding explicitly clears the graph,
passes and frame index with Reset between complete frames. Derived Configure
must recreate generation-specific resources; external passes are re-added.
Execute/Present share one frame index, advanced after Present. Invalid and
reentrant calls fail in Debug and Release. A configuration/frame exception
leaves a failed state until explicit Reset, avoiding retries over a partial
graph. Presentation failure recovery also requires discarding the partially
recorded caller-owned presentation command.

Reset clears graph callbacks before pass owners and does not wait on the
device. Existing backend command/submission retention owns GPU lifetime for
submitted target textures, materials, pipelines and buffers. Rebuild is not
allowed from hooks or between Execute and Present. This is explicit offscreen
target rebuilding, not automatic swap-chain resize or camera stacking.

`RenderPipelineBuilder` wraps graph target/pass creation. `RenderPipelineContext`
carries the graph, current command buffer, camera list, and frame index. This is
a first composable framework, not full Unity URP/HDRP parity: pipeline assets,
camera stacking, post-process stacks, editor authoring, and automatic pipeline
switching are not implemented.

The RenderPipeline example performs a real compute, offscreen graphics, and
sampled presentation chain. `UpdateColorPass` writes a storage buffer,
`GeometryPass` draws into an offscreen target, and `PresentPass` samples that
target into presentation. Debug/Release 120-frame runs passed on OpenGL,
Vulkan, and D3D12 with the frame-119 storage-buffer readback and normal
shutdown. D3D12 used the debug layer; its sampler-creation warnings are benign
and existing.

Lifecycle follow-up verification used an isolated GE_WITH_MONO=OFF CMake build:
Debug and Release engine/example builds and 120-frame OpenGL, Vulkan shared,
D3D12 and Vulkan dedicated runs passed. Debug additionally passed 6,000 frames
per variant. Each run checked three graph generations, paired frame hooks,
configuration failure recovery, dimensions and compute results. D3D12 debug
layer logs had existing sampler warnings, no errors; Debug Vulkan had no
validation errors or validation-unavailable fallback. This does not verify
script integration, interactive window resize, visible/final pixel output or
long-run memory. D3D12 debug-layer handle growth remains an independent open
diagnosis; the bounded run does not attribute allocation ownership.

## Planned extensions

[Pipeline TODO P1-P3](../../../TODO/Rendering/RenderPipeline.md#planned-pipeline-extensions-2026-10-31)
owns the planned asset/switching, camera stacking and post-processing milestones.
The intended flow is asset validation and pipeline selection between complete
frames, explicit base/overlay composition, then ordered effects before final
presentation. Failed selection must preserve the working pipeline; outgoing
GPU resources remain retained through every relevant queue's completion.
Camera attachment load/store/depth dependencies and effect input/output textures
must be declared through RenderGraph, with explicit unsupported capabilities.
This plan introduces no implemented architecture, resource pooling, native API
coupling or per-frame device wait. Existing lifecycle ownership remains unchanged.
