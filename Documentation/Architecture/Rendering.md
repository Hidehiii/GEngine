# Rendering and backend architecture

Editor E0.2 reuses the offscreen RenderGraph and texture-array readback inside
an isolated GEngine child. The OpenGL framebuffer texture wrapper now queries
level-zero dimensions/internal format when adopting its owned texture handle;
copy validation can therefore accept RGBA8 attachments instead of seeing zero
dimensions/uninitialized format. Ownership and engine public signatures are unchanged.
The low-frequency synchronous preview is documented in [Qt editor](QtEditor.md).
Vulkan framebuffer wrappers now carry dimensions and an explicit borrowed-image
flag; the framebuffer still owns their images/views/allocation. Color attachment
allocation includes TRANSFER_SRC for preview copies. Size is no longer used as
an ownership sentinel, preventing double retirement when metadata becomes valid.
Vulkan render-pass creation omits depth references/resolves when no depth
attachment exists and uses color-write access with color-output stages. The
editor's color-only preview exercises this path with validation in Debug.

[Architecture index](Architecture.md) · [Graphics API](../API/GraphicsAPI.md) ·
[Render graph API](../API/RenderGraph.md)

This page is the entry point for renderer ownership, backend boundaries, and
GPU lifetime. Application and renderer code use `GEngine/include/GEngine`
interfaces. Native handles, allocation policies, and synchronization remain in
backend code.

## Topic index

- [Frame work and graph](Rendering/FrameWork.md)
- [Render pipeline framework](Rendering/RenderPipeline.md)
- [Device queues and presentation](Rendering/Queues.md)
- [Backend layering and shader ownership](Rendering/Backends.md)
- [GPU resource lifetime](Rendering/Lifetime.md)
- [Extension points](Rendering/Extensions.md)

## Frame work and graph

`RenderSystem` owns the frame and imports the selected swap-chain image for
presentation. `RenderGraph` expresses pass ordering, resource versions,
resource states, and subresource ranges. D3D12 records resource/UAV barriers,
Vulkan records image/buffer/memory barriers, and OpenGL emits memory barriers
without tracking image layouts.
