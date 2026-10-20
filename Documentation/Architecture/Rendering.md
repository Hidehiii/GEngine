# Rendering and backend architecture

[Architecture index](Architecture.md) · [Graphics API](../API/GraphicsAPI.md) ·
[Render graph API](../API/RenderGraph.md)

This page is the entry point for renderer ownership, backend boundaries, and
GPU lifetime. Application and renderer code use `GEngine/include/GEngine`
interfaces. Native handles, allocation policies, and synchronization remain in
backend code.

## Topic index

- [Frame work and graph](Rendering/FrameWork.md)
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
