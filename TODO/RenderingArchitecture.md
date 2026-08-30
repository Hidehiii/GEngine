# Rendering architecture refactor

## Goal

Provide one portable rendering API for OpenGL, Vulkan, and D3D12.  Renderer
code must describe work and resource usage without referencing native graphics
types.  Each backend is responsible for translating that portable description
to its own synchronization and submission model.

## Architecture target

```text
Application / render features
            |
       Frame graph
            |
 Device + Queue + Command encoder + Swapchain
            |
    OpenGL | Vulkan | D3D12
```

## Phase 1 — remove cross-backend leaks (current)

- [x] Move resource-state definitions into the common graphics API.
- [x] Remove D3D12 headers, resource casts, and barrier translation from
      `RenderSystem`.
- [x] Give every backend a common resource-transition entry point.
- [ ] Make graph resources hold typed engine resources rather than `void*`.
- [ ] Route the presentation target through the same graph-resource path.
- [ ] Make OpenGL and Vulkan implement a deliberate transition policy; they
      must not silently skip graph declarations.

Acceptance: `GEngine/src/GEngine/Renderer` contains no `D3D12`, `Vulkan`, or
`OpenGL` symbols or includes.  The Triangle presentation path remains valid on
all three APIs.

## Phase 2 — explicit frame ownership

- [x] Replace `GraphicsPresent::s_CommandBuffers` with per-presenter members.
- [ ] Replace global `Graphics` backend ownership with an `EngineRuntime` owned
      device instance and deterministic shutdown.
- [ ] Split command recording from queue submission.  `CommandEncoder::End()`
      records only; `Queue::Submit()` performs synchronization and submission.
- [ ] Represent acquire/present semaphores or fences through the swapchain and
      queue interfaces instead of command-buffer side lists.

Acceptance: recreating a runtime does not reuse command buffers or backend
state from the previous runtime.

## Phase 3 — real frame graph

- [ ] Add typed image/buffer descriptors and transient resource creation.
- [ ] Track resource versions, reads, writes, first/last use, and final state.
- [ ] Add stage/access intent, subresource ranges, queue ownership, and
      per-backend barrier compilation.
- [ ] Add resource pooling and aliasing only after lifetime tracking is tested.
- [ ] Expose pass builders to renderer features so off-screen work is declared
      instead of hidden in `Layer::OnRender`.

Acceptance: a graphics -> compute -> graphics chain can run without any
backend-specific barrier code in a layer or renderer feature.

## Phase 4 — device-owned creation and modules

- [ ] Move Shader, Material, Sampler, Pipeline, texture, and compute factories
      under `RenderDevice`; remove central API switches from common sources.
- [ ] Build OpenGL, Vulkan, and D3D12 as separate backend modules.
- [ ] Replace the Vulkan-only subpass public contract with portable graph pass
      attachments; retain a legacy adapter only where needed.
- [ ] Make capabilities data-driven from queried device features, not hardcoded
      booleans or conservative constants.

Acceptance: adding a backend requires a backend module and registration only;
it does not require editing every common resource factory.

## Verification matrix

For every phase, run the Triangle example on all available APIs and verify:

| Check | OpenGL | Vulkan | D3D12 |
| --- | --- | --- | --- |
| Acquire / present | pending | pending | pending |
| `float3` vertex input | pending | pending | pending |
| Repeated-frame memory stability | pending | pending | pending |
| Validation / debug output clean | pending | pending | pending |
