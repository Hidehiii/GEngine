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

- [~] Fix OpenGL state initialization before a current context/GLAD loader.
      GraphicsRuntime constructs the backend during Configure, before window
      creation. Move GL state setup from its constructor to the context init path
      after successful loader initialization. Reject loader failure outside
      assertions. Acceptance: OpenGL FrameGraphTriangle starts past glEnable and
      exercises replacement without GL errors. Other failures stay separately tracked.
      Follow-up found during verification: compiler emits Vulkan 1.3 SPIR-V,
      but OpenGL passes it directly to glShaderBinary, which rejects it. Convert
      that intermediate to desktop GLSL with existing SPIRV-Cross dependency,
      compile per stage with useful errors, and clean up partial programs on
      failure. Verify FrameGraphTriangle; complex resource bindings remain separate.
      Next exposed path: data-backed OpenGL vertex buffers never create a VAO,
      Bind dereferences a missing index buffer for non-indexed draws, attribute
      size uses bytes instead of component count, and destruction leaks the VAO.
      Fix these together; acceptance: float3 non-indexed replacement renders
      without GL errors or access violations and VAOs are reclaimed.
      Implemented 2026-09-12. Debug build passed. Final OpenGL 4.6 / NVIDIA
      RTX 3060 Laptop GPU run lasted 30 seconds without GL_INVALID errors,
      assertions or access violations; private bytes: 169664512 / 169664512 /
      169684992. Driver emitted shader-recompilation performance warnings.
      Earlier runs exposed and reproduced the SPIR-V and VAO failures above.
      Process stopped at test limit: visible output, resize, normal shutdown,
      long-duration memory and complex resource-binding cases remain unverified.
      Release not rebuilt because the tracked PhysX configuration blocker remains.
      User confirmed OpenGL runs without the reported problem on 2026-09-12.
      The driver shader-recompilation performance warning was not a render failure;
      this confirmation does not close the remaining stress/Release checks.

- [~] Fix D3D12 in-flight pipeline release during FrameGraphTriangle replacement.
      Reproduced with debug layer: final-release of ID3D12PipelineState while
      referenced by GPU work; Present subsequently fails. Retain draw/dispatch
      owners in command buffers, track completion using a per-buffer monotonic
      fence, and wait only before allocator reuse/destruction. Cover offscreen
      and presentation recording; retain transitioned resources as well.
      Acceptance: Debug build and D3D12 debug-layer replacement run without
      in-flight release diagnostics, bounded memory. In-place resource mutation
      and multi-queue stress need separate coverage. Label expected cycle test.
      Implemented 2026-09-12. Debug engine/example build passed (1586 warnings,
      zero errors). D3D12 debug-layer 30-second run no longer reported the
      previously reproduced PSO final-release corruption; no D3D12 warnings or
      errors captured. Private bytes at 10/20/30 s: 184463360 / 185028608 /
      185348096. This small increase is not proof of long-term bounded memory.
      Agent-owned test stopped at its time limit; normal shutdown, resize,
      prolonged runs, compute/transfer stress and Release remain unverified.
      User confirmed D3D12 runs without the reported error on 2026-09-12;
      this does not establish coverage of every pending scenario.

- [~] Avoid backend shader construction on repeated factory cache hits.
      Scope: pre-construction cache keyed by active API and normalized absolute
      source path, shared by all three backends; clear it at runtime shutdown.
      Preserve existing first-loaded shader-name lookup behavior. This is not
      hot reload or concurrent loading. Acceptance: repeated and dot-segment
      path calls reuse the shader without repeated load logs; Debug build and
      bounded Vulkan run with memory samples. Other backend runtime checks remain
      independent. Add the regression to FrameGraphTriangle and document semantics.
      Implemented 2026-09-12. Debug engine/example build: zero errors. Vulkan
      30-second run: exactly one initial shader-load log despite 64 startup
      requests and periodic material replacement. Private bytes at 10/20/30 s:
      401768448 / 401768448 / 401772544. No validation errors in captured output;
      expected graph-cycle negative test logged. Agent-owned process stopped at
      the time limit; shutdown, resize, long-run memory, cache-clear/reload and
      OpenGL/D3D12 runtime checks remain pending. Release was not rebuilt; its
      tracked PhysX configuration blocker is outside this change.

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
        Current milestone: retire material descriptor sets and shader-owned
        layouts/modules through submission completion; make shared-pool release
        null-safe and keep it after the shutdown idle/retirement flush. Extend
        FrameGraphTriangle to replace materials as well as pipelines/buffers.
        Acceptance: Debug build, repeated replacement/resize/shutdown without
        descriptor lifetime errors or unbounded growth. Runtime checks pending.
        Inspection found Shader::Create constructs a backend shader before its
        cache lookup. Use GetShader in the replacement example to isolate material
        retirement; follow-up: avoid eager construction on cache hits and verify
        repeated factory calls do not cause memory growth. The initial Vulkan
        smoke run showed rising private bytes; the cause is not yet established.
        Verification 2026-09-12: Debug engine/example build passed (zero errors).
        Final Vulkan example ran for 30 seconds on RTX 3060 Laptop GPU; private
        bytes at 10/20/30 seconds: 404291584 / 404422656 / 404422656. No validation
        errors found in captured output; expected graph-cycle negative test logged.
        Agent-owned process was stopped after the bounded run. An earlier normal
        close request timed out, so shutdown and resize are NOT verified. Visible
        output, long-duration memory, OpenGL/D3D12, and Release remain unverified;
          the existing Release/PhysX configuration blocker remains open.
      Submission-serial retirement is implemented and graphics/compute pipeline
      replacements, core buffer/image/sampler objects, and framebuffers use
        it. Material descriptor sets and shader layouts/modules now use it too.
        The shared descriptor pool remains context-owned and is destroyed after
        the shutdown flush; release now resets its initialized null handle.
        Less common resource types and shader-replacement stress still need review.
      Queue submissions use dedicated completion fences and a contiguous
      watermark. Resource owners must survive recording until submission.
        FrameGraphTriangle now exercises repeated buffer/pipeline/material replacement;
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
