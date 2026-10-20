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
