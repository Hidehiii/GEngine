## Resources and states

Foundation preflight follow-up (2026-10-31): portable/legacy target
validation and device-limit rejection before allocation are tracked in
[Phase 4](../../../TODO/Rendering/Phase4.md). No new target creation overload is
planned; the legacy adapter remains supported.

Both target overloads reject missing/invalid color formats (including depth in
color slots), invalid depth formats, zero/non-power-of-two sample counts or
sample counts above 64, invalid load/store enums and invalid subpass indices at
declaration time. Samples passing this structural check still require native
format/sample support; it is not a device support guarantee. Input attachment
indices currently address colors only. A rejected declaration adds no target
or color-resource handles. During Compile, all targets are checked against
framebuffer/render-pass capability, texture dimensions, MaxColorAttachments
and subpass support before any new graph resource or target is allocated.
Compatibility callers retain their legacy native dependency descriptions.

These initial generic checks passed in FrameGraphTriangle on OpenGL, Vulkan
shared/dedicated and D3D12 in CMake Debug/Release, including 26 malformed
declarations and allocation-free device rejection. The 2026-11-01 implementation
below adds native format/MSAA and framebuffer bounds. The compatibility adapter
has not been removed; high-count/depth-sampling coverage remains limited.

Per-format MSAA validation (2026-11-01) now calls the device's
QueryRenderTargetFormat/ValidateRenderTarget path for every target before new
transient or target allocation. Color/depth sample counts must intersect for
the requested count; automatic color resolve must be supported. Dimensions
also obey format/framebuffer limits. Direct and legacy target creation follow
the same native capability checks. Structural counts alone do not imply
support; unsupported combinations throw without downgrade. Graph color
resources refer to resolved single-sample textures. Depth resolve and preserved
MSAA LOAD limitations are specified in [device capabilities](../Graphics/Device.md).

When reusing a graph before its external completion command is submitted,
declare the final graphics readers that join copy/transfer output back to the
next graphics use. The MSAA regression includes this explicit sampled-read join;
separate graphs do not infer dependencies from sharing CPU resource objects.

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

## MSAA depth and preserved LOAD follow-up (2026-11-01)

AttachmentSpecification.DepthResolve forwards DepthResolveMode to the native
render pass, including through the legacy declaration adapter. Device preflight
rejects unavailable modes before graph allocation. Changing load operations on
the compiled framebuffer retains backing images; Vulkan now stores MSAA content
for subsequent LOAD. STORE must precede LOAD; recreated targets have no previous
content. Verification is tracked in Phase 4. The graph does not infer ordering
between independently created graphs; explicit graphics read joins remain needed
in diagnostics that repeat execution before external frame completion.
Debug/Release four-backend depth/LOAD matrices and Debug 6000-frame runs passed;
the example checks actual-frame retention and replacement as well as repeated
execution. Final evidence and untested variants are recorded in Phase 4.
