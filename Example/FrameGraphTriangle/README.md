# FrameGraphTriangle

This is a visual smoke test for the portable presentation path.  It renders an
orange triangle through only public engine APIs; it does not include D3D12,
Vulkan, or OpenGL headers and it does not access native handles.

The layer submits one off-screen graphics command buffer through the public
`GraphicsQueue`, then makes the presentation command buffer depend on it. The
engine's `RenderSystem` builds the following per-frame graph around the layer
draw:

```text
LayerRender (off-screen triangle, queue submit)
    -> Present (visible triangle draw) -> ReleasePresentation
                                      Present -> RenderTarget -> Present
```

Run the project after selecting the desired `GraphicsAPI` in `Config.ini`.
Close it after several seconds as a quick check that command buffers, presenter
fences/events, and cached render resources are released while the graphics
context is still alive.  It also exercises the record-then-submit frame path:
drawing ends command recording first, and the presenter submits it only after
swapchain synchronization is assembled. On Vulkan, this additionally verifies
that the presentation submit waits for the acquire semaphore and the off-screen
submission dependency without keeping synchronization state in a command
buffer.

For non-presentation work, submit a completed command buffer through the
portable device queue:

```cpp
auto& queue = Graphics::GetRenderDevice().GetQueue(COMMAND_BUFFER_TYPE_GRAPHICS);
queue.Submit(commandBuffer);
```

## Resource replacement check

Startup checks also cover reverse access declaration order, repeated graph
compilation, dependency-cycle detection, transient read-before-write, and invalid
render-target usage. One expected cycle diagnostic is printed by the negative
test. Failure throws before drawing; these checks do not validate GPU barriers.

The triangle alternates left/right every 30 rendered frames while replacing its
pipeline and vertex buffer. Run for several minutes, resize the window, then
close it. Check for stable memory and clean backend validation output. Test each
API independently. Startup also checks implicit RenderGraph compilation without
assertions, so run both Debug and Release. Build success alone is not a runtime
validation result.

The non-instanced replacement path also checks optional Vulkan allocation
lifetime: no instance buffer is allocated, and retiring the vertex buffer must
not free an uninitialized instance-memory handle. Check that repeated movement
and shutdown produce no `vkFreeMemory` validation errors.
