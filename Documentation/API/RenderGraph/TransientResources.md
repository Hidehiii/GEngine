## Transient resources

Frame-local working resources can be declared without creating a backend object
in feature code. The graph creates them when it compiles, and their lifetime
ends when `Reset()` is called:

```cpp
const auto lighting = graph.CreateTransientTexture2D(
    "Lighting",
    { viewportWidth, viewportHeight, 1, 1, RENDER_IMAGE_2D_FORMAT_RGBA8_UNORM });

graph.Write(uploadPass, lighting, RenderGraph::ResourceState::CopyDestination);
graph.Read(postProcess, lighting, RenderGraph::ResourceState::ShaderRead);

// Inside a pass callback after compilation:
auto texture = graph.GetTexture2D(lighting);
```

This texture path supports sampling and copies, not attachment rendering.
`Texture2DDesc` declares `MipLevelCount` and `ArrayLayerCount`; a 2D texture
currently requires one array layer. `Texture2DArrayDesc` declares its layer
count (with one mip level until backend support is expanded), and
`StorageImage2DDesc` declares its subresource counts (currently one mip and
one layer). Those counts are the compile-time source for resolving `All`.
The upload pass must actually initialize the texture contents. Use an imported
framebuffer attachment for render-target work. Usage-aware transient attachment
creation remains open in
[RenderingArchitecture.md](../../../TODO/RenderingArchitecture.md).
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
memory reuse is not enabled. Pooling and aliasing are tracked only in
[RenderingArchitecture.md](../../../TODO/RenderingArchitecture.md).

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
