## Presentation integration

`RenderSystem` imports the selected swap-chain image as `PresentationBackBuffer` for every frame. It records a `Present -> RenderTarget` transition before layer presentation and a final `RenderTarget -> Present` transition before submission.

Layers normally should not manipulate that swap-chain state directly. Draw through `GraphicsPresent::Render` from `Layer::OnPresent`; the renderer owns target binding, clearing, transitions, command-list close and presentation.

For Vulkan, the legacy swap-chain render pass currently owns the native
`Present <-> ColorAttachment` image-layout transitions. The graph still tracks
the presentation resource for ordering, but does not emit a duplicate native
barrier for that image. This is an implementation boundary; layer code remains
backend-neutral.

## Rules

- Declare each resource read/write access in the graph instead of adding backend-only state transitions inside a layer.
- Make write-after-read and write-after-write operations explicit through `Read`/`Write`; the graph derives ordering dependencies from them.
- Reset the graph every frame before creating that frame's passes.
- Keep pass callbacks focused on recording commands. Long-lived ownership
  belongs outside the graph; use transient descriptors only for frame-local
  working resources.
