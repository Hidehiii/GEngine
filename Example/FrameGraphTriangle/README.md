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
