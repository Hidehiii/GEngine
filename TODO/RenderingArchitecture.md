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
- [x] Make graph resources hold typed engine resources rather than `void*`.
- [x] Route explicit presentation targets through the same graph-resource path.
- [x] Make OpenGL and Vulkan implement a deliberate transition policy.  OpenGL
      emits memory barriers; Vulkan emits image/buffer barriers for engine
      resources, while its legacy swapchain render pass explicitly owns its
      presentation-image layout transitions.

Acceptance: `GEngine/src/GEngine/Renderer` contains no `D3D12`, `Vulkan`, or
`OpenGL` symbols or includes.  The Triangle presentation path remains valid on
all three APIs.

## Phase 2 — explicit frame ownership

- [x] Replace `GraphicsPresent::s_CommandBuffers` with per-presenter members.
- [x] Release presenter-owned fences/events and static render-resource caches
      before their window-owned graphics context is destroyed.
- [x] Move backend ownership and per-runtime frame state into a
      `GraphicsRuntime` owned by `RenderSystem`.  `Graphics` is now a
      non-owning compatibility facade for the active runtime during migration.
- [x] Make `CommandBuffer::End()` record only.  Device submission now goes
      through `Graphics::SubmitCommandBuffer`, while the presenter submits its
      recorded command buffer with swapchain synchronization.
- [x] Wait for all queues before layer-owned resources are released during
      runtime shutdown, then destroy Vulkan swapchain and ImGui render-pass
      resources before the device.
- [x] Promote device submission to public `GraphicsQueue::Submit()` objects.
      Callers can select a graphics, compute, or transfer queue through
      `RenderDevice` without using the legacy `Graphics` facade.
- [x] Keep Vulkan acquire/present semaphores and frame fences in the
      swapchain presenter, and keep inter-command dependencies in the device
      submission layer instead of storing synchronization lists in command
      buffers.
- [ ] Complete Vulkan deferred deletion for all replaceable GPU resources.
      Submission-serial retirement is implemented and graphics/compute pipeline
      replacements, core buffer/image/sampler objects, and framebuffers use
      it. Descriptor pools, descriptor-set layouts, and less common resource
      types still need to be migrated to the same lifetime path.
      Queue submissions use dedicated completion fences and a contiguous
      watermark. Resource owners must survive recording until submission.
      FrameGraphTriangle now exercises repeated buffer/pipeline replacement;
      runtime validation on all three APIs remains pending.

Acceptance: recreating a runtime does not reuse command buffers or backend
state from the previous runtime.

## Smoke-test example

`Example/FrameGraphTriangle` is a visible, cross-API smoke test for the work
completed in phases 1 and 2.  It draws an orange triangle through the public
engine API, while `RenderSystem` owns the graph declaration and portable
presentation resource transitions.  The example has no platform includes and
no native handle access.

Run it after selecting the backend in `Example/FrameGraphTriangle/Config.ini`:

| Value | Backend |
| --- | --- |
| `1` | OpenGL |
| `2` | Vulkan |
| `3` | D3D12 |

Keep it open for several seconds and close it to exercise both repeated frame
submission and deterministic renderer shutdown.

## Phase 3 — real frame graph

- [ ] Make graph validation deterministic before GPU allocation: rebuild inferred
      dependencies at compile time independently of access declaration order,
      reject incompatible transient resource states and read-before-write, and
      exercise these rules in FrameGraphTriangle startup checks. Pass creation
      order defines unversioned resource order. Full usage-aware attachment
      creation and backend memory-barrier compilation remain separate work.
      Implementation and startup regression scenarios are present; execution
      of the scenarios and backend runtime verification remain pending.
      Debug engine and FrameGraphTriangle build passed with zero errors on
      2026-09-10. This does not count as executing the regression scenarios.

- [~] Add typed image/buffer descriptors and transient resource creation.
      `RenderGraph` can now create transient 2D textures, storage buffers, and
      storage images from portable descriptors. Pooling and aliasing remain
      intentionally deferred until lifetime analysis is in place.
- [ ] Track resource versions, reads, writes, first/last use, and final state.
      First/last use and final state are now recorded for every compiled
      resource. Versioned handles and explicit read/write version propagation
      are still required before aliasing can be enabled.
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

- [ ] Fix Release build definitions: PhysX requires exactly one of `NDEBUG`
      and `_DEBUG`; the current Release configuration fails with C1189.
      Re-run the assertion-disabled FrameGraphTriangle startup check afterward.

For every phase, run the Triangle example on all available APIs and verify:

| Check | OpenGL | Vulkan | D3D12 |
| --- | --- | --- | --- |
| Acquire / present | pending | pending | pending |
| `float3` vertex input | pending | pending | pending |
| Repeated-frame memory stability | pending | pending | pending |
| Validation / debug output clean | pending | pending | pending |

Current local build note: automated compilation must run from a clean Visual
Studio developer environment.  The current host process exports both `PATH`
and `Path`, which causes MSBuild to fail before `CL.exe` receives any source.
