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

The graph adds ordering dependencies for conflicting accesses and inserts a
transition whenever a resource's requested state differs from its previous
state. Read/write declarations must match the state: `ShaderRead` and
`CopySource` are read-only, while `RenderTarget`, `DepthWrite`,
`CopyDestination`, and `Present` are declared as writes. `ShaderWrite` is the
storage/UAV state and can be declared with either read or write intent. Stage
declarations must also match the state: shader states accept graphics or
compute, copy states require transfer, and attachment/presentation states
require graphics. Fine-grained graphics stages are available as
`VertexShader`, `FragmentShader`, `ColorOutput`, and `DepthAttachment`.
Shader states accept vertex or fragment; RenderTarget accepts ColorOutput;
DepthWrite accepts DepthAttachment. These values are legal only on graphics
passes. Vulkan maps them to native stage masks. D3D12 narrows ShaderRead to
NON_PIXEL_SHADER_RESOURCE for vertex/compute, PIXEL_SHADER_RESOURCE for
fragment, and ALL_SHADER_RESOURCE for Graphics/All. OpenGL accepts the
declarations but conservatively uses whole-resource memory barriers.
When only the declared stage changes while the logical state stays the same,
Vulkan records a memory dependency, D3D12 transitions between pixel/non-pixel
shader-resource states where applicable, and OpenGL conservatively emits its
state-appropriate memory barrier.

For a graphics-queue copy without a render target, use
`BuildGraphicsCopyPass(name, callback)`. The callback receives a graphics
command buffer but does not begin a render pass. Declare both sides of the
copy before `ExecuteGpu`; for example, a `Texture2D` source can be read in
`CopySource` and one `Texture2DArray` layer written in `CopyDestination` with
`GraphicsPipelineStage::Transfer`. Call
`Graphics::GetRenderDevice().CopyTexture2DToArrayLayer(command, source, array,
layer)` inside the callback. Source and destination must have matching
dimensions and format, and the layer must exist. Declare a later shader read
to restore the resources to `ShaderRead`. `Texture2DArray::ReadLayerData` is a
synchronous diagnostic readback for an exact-size RGBA8 layer, not a per-frame
streaming path. The FrameGraphTriangle example checks that the copied layer
changes while its neighboring layer remains intact.
