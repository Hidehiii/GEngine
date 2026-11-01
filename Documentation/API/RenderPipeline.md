# Render pipeline API

Lifecycle follow-up (2026-10-31): implementation and verification status are
tracked in [the pipeline TODO](../../TODO/Rendering/RenderPipeline.md).

[API index](README.md) · [Render pipeline architecture](../Architecture/Rendering/RenderPipeline.md)

Include `GEngine/Renderer/RenderPipeline.h`. The abandoned
`GEngine/Graphics/RenderPipeline.h` two-method interface is removed.

## Core types

```cpp
class RenderPipelinePass
{
public:
    virtual const std::string& GetName() const = 0;
    virtual void Configure(RenderPipelineBuilder& builder) = 0;
    virtual void Execute(RenderPipelineContext& context) = 0;
    virtual void Present(RenderPipelineContext& context);
};
```

`RenderPipeline` owns a `RenderGraph` and ordered pass list:

```cpp
RenderPipeline pipeline;
pipeline.AddPass(pass);
pipeline.Execute(GraphicsPresent::GetCommandBuffer(), cameras);
pipeline.Present(GraphicsPresent::GetCommandBuffer(), cameras);
```

Derived pipelines normally override `Configure(RenderPipelineBuilder&)` to
create targets/resources and add passes. The framework then calls every
registered pass's `Configure` exactly once in insertion order, including passes
added externally before first execution. Do not configure passes manually in
the derived hook. If a pass consumes another pass's output version, resolve it
in the consuming pass's `Configure`, after the producer has configured.
`RenderPipelineBuilder` wraps graph
target creation and graphics/compute/transfer pass creation.
`RenderPipelineContext` exposes the graph, current command buffer, cameras, and
zero-based frame index. Each successful `Execute` must be followed by exactly
one `Present`; both receive the same frame index. The index advances only after
successful presentation. Present-before-execute, double execute/present and
reentrant phase calls throw `std::logic_error`, including Release builds.

`AddPass` and `RemovePass` are valid only before configuration or inside the
pipeline's configuration hook. Once configured, call `Reset()` between complete
frames before changing passes or target dimensions. Reset clears graph, passes
and frame index; the next Execute invokes the derived Configure again. External
passes must be re-added. Reset inside hooks or between Execute and Present is
rejected. A hook/graph exception marks the pipeline failed; subsequent frame
calls are rejected until Reset. Callers must abandon any partially recorded
presentation command when handling a presentation failure. Null command buffers
and invalid pass registrations are rejected independently of assertions.

The graph and pass callbacks own CPU resource references. Submitted command
resources remain retained by the backends until completion; Reset is not a GPU
wait. Direct graph access must respect the same between-frame rebuild boundary.

The example in `Example/RenderPipeline` demonstrates a complete compute,
offscreen graphics, and sampled presentation pipeline. It verifies the
storage-buffer result at frames 29, 59 and 119, rebuilds offscreen dimensions
at frames 30 and 60, and exits normally at frame 120. This is a
first composable-pipeline milestone; it does not provide full URP/HDRP pipeline
assets, camera stacking, post-processing stacks, or editor authoring.

The lifecycle/rebuild checks passed in isolated GE_WITH_MONO=OFF CMake Debug
and Release builds on OpenGL, Vulkan shared/dedicated queues and D3D12 for
120 frames each; Debug also passed 6,000 frames per variant. These results
cover the frame/configuration contract and numeric compute result, not final
presentation pixels, actual window interactions or script-enabled integration.

## Planned extensions

[Pipeline TODO P1-P3](../../TODO/Rendering/RenderPipeline.md#planned-pipeline-extensions-2026-10-31)
tracks pipeline assets/runtime switching, explicit base/overlay camera stacks,
and an ordered post-processing stack starting with exposure and tone mapping.
These are unimplemented plans; this documentation update adds no API or asset
format. Current configuration uses C++, Cameras supplies data without automatic
stack composition, and sampled presentation does not provide effect processing.
Future contracts must specify failed-switch rollback, camera attachment rules
and intermediate texture capabilities before public signatures are introduced.
