# Render pipeline framework

[Rendering architecture](../Rendering.md) · [Render pipeline API](../../API/RenderPipeline.md)

`RenderPipeline` is the backend-neutral composable pipeline layer above
`RenderGraph`. The old two-method `Graphics/RenderPipeline` contract is removed.
The new public header is `GEngine/Renderer/RenderPipeline.h`.

A pipeline owns one persistent `RenderGraph`, an ordered list of
`RenderPipelinePass` objects, and an explicit configure/execute/present flow:

1. A derived pipeline overrides `Configure(RenderPipelineBuilder&)`.
2. It creates targets/resources and adds ordered pass objects.
3. Each pass can build graph passes and declare resource versions.
4. `Execute(completion, cameras)` calls per-frame pass hooks and then executes
   the graph through `RenderGraph::ExecuteGpu`.
5. `Present(command, cameras)` calls pass presentation hooks. Presentation
   remains backend-neutral through `GraphicsPresent`.

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
