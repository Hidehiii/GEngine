# Render pipeline API

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
create targets/resources and add passes. `RenderPipelineBuilder` wraps graph
target creation and graphics/compute/transfer pass creation.
`RenderPipelineContext` exposes the graph, current command buffer, cameras, and
frame index. `Reset()` clears the graph and pass list; the derived pipeline must
be configured again before execution.

The example in `Example/RenderPipeline` demonstrates a complete compute,
offscreen graphics, and sampled presentation pipeline. It verifies the
storage-buffer result at frame 119 and exits normally at frame 120. This is a
first composable-pipeline milestone; it does not provide full URP/HDRP pipeline
assets, camera stacking, post-processing stacks, or editor authoring.
