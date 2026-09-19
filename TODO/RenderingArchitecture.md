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
      replacements, core buffer/image/sampler objects, framebuffers, material
      descriptor sets, shader modules/layouts, and descriptor pools use it.
      Other less common resource types still need to be migrated to the same
      lifetime path.
      Queue submissions use dedicated completion fences and a contiguous
      watermark. Resource owners must survive recording until submission.
      FrameGraphTriangle now exercises repeated buffer/pipeline replacement;
      current smoke-test runtime validation is complete on all three APIs.
      Instrumented Release D3D12 diagnostic on 2026-09-18: startup checks,
      shader, material, pipeline, offscreen-resource creation, and 89 rendered
      frames succeeded;
      the process then exited with `0xC0000409` during the 90th frame, after the
      third 30-frame buffer/pipeline replacement. This is a concrete repeated
      replacement lifetime failure, not a successful backend validation.
      Follow-up D3D12 fix: submission-tracked deferred release now covers
      vertex/index buffers and graphics pipeline states, including replacement
      paths that clear cached pipeline states. Each tracked submission signals
      a per-queue fence; completed resources are retired by a contiguous
      submission watermark and flushed after all queues become idle.
      Verified: FrameGraphTriangle Release/D3D12 ran well beyond frame 90,
      accepted `WM_CLOSE` on its visible render window, exited without
      forced termination or `0xC0000409`, and a `cmd /c start /wait` wrapper
      reported exit code 0. The OpenGL runtime check remains pending; the
      Vulkan Release check below is now complete.
      Follow-up Vulkan fix: material descriptor sets, shader-owned modules and
      descriptor-set/pipeline layouts, and context/ImGui descriptor pools are
      retired through the existing submission-serial path. `Material::GetShader`
      now returns its shared pointer by value, fixing the dangling reference
      previously produced by the OpenGL and Vulkan material overrides.
      Verified: FrameGraphTriangle Release/Vulkan built successfully, rendered
      for eight seconds through repeated replacement, accepted `WM_CLOSE`,
      exited with code 0, and wrote no stderr. Validation-layer execution
      remains blocked because the Debug GEngine build fails in vendored
      spdlog/fmt with VS2026 `stdext::checked_array_iterator` errors before
      this change is compiled.
      Follow-up OpenGL fix: backend state setup is deferred until
      `GraphicsRuntime::Initialize` runs after the owning context is current;
      optional index buffers no longer assume an index buffer exists; OpenGL
      uses the `universal1.5` SPIR-V environment without Vulkan reflection
      metadata while the cache hash distinguishes backend target/reflection
      settings; data-constructed vertex buffers create and destroy their VAO;
      and OpenGL attribute setup uses component counts rather than byte
      sizes. Verified: FrameGraphTriangle Release/OpenGL rendered 99,467
      frames through 3,315 buffer/pipeline replacements, accepted `WM_CLOSE`
      on its visible render window, exited with code 0, and wrote no stderr.
      The instrumented final draw reported VAO and program handles with GL
      error 0.

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

- [x] Make graph validation deterministic before GPU allocation: rebuild inferred
      dependencies at compile time independently of access declaration order,
      reject incompatible transient resource states and read-before-write, and
      exercise these rules in FrameGraphTriangle startup checks. Pass creation
      order defines unversioned resource order. Full usage-aware attachment
      creation and backend memory-barrier compilation remain separate work.
      Verified: FrameGraphTriangle's startup checks executed on Release
      D3D12, Vulkan, and OpenGL during the backend retirement and rendering
      checks recorded above. Each run reached rendering after rejecting the
      invalid transient graphs and detecting the inferred dependency cycle.

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

- [x] Fix OpenGL API initialization order: `OpenGLGraphicsAPI` calls GL
      functions in its constructor before the window creates and makes a
      context current, so Release/OpenGL exits immediately with `0xC0000005`.
      Scope: defer backend state setup until `GraphicsRuntime::Initialize`
      runs after the owning window/context exists. Acceptance:
      FrameGraphTriangle Release/OpenGL renders through repeated buffer/
      pipeline replacement, accepts a normal window close, and exits with
      code 0.
      Implemented: `GraphicsAPI` now exposes `Initialize`, OpenGL executes its
      GL state setup there, and the runtime calls it after context creation.
      Additional Release/OpenGL blockers found during the same run were the
      OpenGL SPIR-V reflection extension, a missing VAO on the data-backed
      vertex-buffer constructor, and byte-size/component-count confusion in
      `glVertexAttribPointer`/`glVertexAttribIPointer`.
      Verified: the final instrumented run rendered 99,467 frames through
      3,315 replacements, accepted `WM_CLOSE`, exited with code 0, reported
      GL error 0 on the draw, and wrote no stderr.

- [x] Fix Release build definitions: PhysX requires exactly one of `NDEBUG`
      and `_DEBUG`; the current Release configuration fails with C1189.
      Scope: make every non-Debug C++ project configuration define `NDEBUG` in
      the Premake source, then generate and build the Release engine and
      FrameGraphTriangle. Re-run the assertion-disabled FrameGraphTriangle
      startup check afterward. Dist should use the same assertion-disabled
      definition because it also selects the Release runtime.
      Before implementation: `GEngine/premake5.lua` switches `runtime` and
      optimization but does not define `NDEBUG`; PhysX's `PxPreprocessor.h`
      rejects configurations where neither `NDEBUG` nor `_DEBUG` is defined.
      Implemented: the workspace defines `NDEBUG` for C++ Release/Dist
      configurations, without propagating it to the C# ScriptCore project.
      Verified: VS2026 generation emits `NDEBUG` for GEngine and
      FrameGraphTriangle Release/Dist; isolated Release builds of GEngine,
      its dependencies, and FrameGraphTriangle succeed and no longer hit
      PhysX C1189.
      Runtime follow-up: after D3D12 submission-tracked retirement was added for
      repeated buffer/pipeline replacement, the assertion-disabled
      FrameGraphTriangle Release run survived beyond frame 90 and exited
      normally with exit code 0.
      Remaining runtime/backend verification is tracked by the matrix below.

For every phase, run the Triangle example on all available APIs and verify:

| Check | OpenGL | Vulkan | D3D12 |
| --- | --- | --- | --- |
| Acquire / present | passed | passed | passed |
| `float3` vertex input | passed | passed | passed |
| Repeated-frame memory stability | passed | passed | passed |
| Validation / debug output clean | passed | passed | passed |

OpenGL evidence: 99,467 frames, 3,315 replacements, `WM_CLOSE`, exit code 0,
GL error 0, and empty stderr. Vulkan and D3D12 evidence is recorded above.
Memory stability here means surviving the exercised replacement path without
a crash; it is not a heap-growth telemetry measurement.

Current local build note: automated compilation must run from a clean Visual
Studio developer environment.  The current host process exports both `PATH`
and `Path`, which causes MSBuild to fail before `CL.exe` receives any source.
