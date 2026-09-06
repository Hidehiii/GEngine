# Render graph

`RenderGraph` is the backend-neutral scheduler for a frame. It owns pass dependencies, per-resource access declarations and the resulting state transitions. It does not own application resources; imported resources remain owned by their texture, buffer or presentation system.

## Basic pattern

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

`AddDependency(after, before)` means that `after` cannot run until `before` has completed.

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
- Keep pass callbacks focused on recording commands. Resource creation and long-lived ownership belong outside the graph.
