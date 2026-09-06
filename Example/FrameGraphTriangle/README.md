# FrameGraphTriangle

This is a visual smoke test for the portable presentation path.  It renders an
orange triangle through only public engine APIs; it does not include D3D12,
Vulkan, or OpenGL headers and it does not access native handles.

The engine's `RenderSystem` builds the following per-frame graph around the
layer draw:

```text
LayerRender -> Present (triangle draw) -> ReleasePresentation
                         Present -> RenderTarget -> Present
```

Run the project after selecting the desired `GraphicsAPI` in `Config.ini`.
Close it after several seconds as a quick check that command buffers, presenter
fences/events, and cached render resources are released while the graphics
context is still alive.  It also exercises the record-then-submit frame path:
drawing ends command recording first, and the presenter submits it only after
swapchain synchronization is assembled.
