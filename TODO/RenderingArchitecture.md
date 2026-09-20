# Rendering architecture refactor

## Integration status (2026-09-19)

Local and remote milestone histories below are retained for traceability.
Their old limitations and results describe the respective parent revisions.
See [CrossComputerIntegration.md](CrossComputerIntegration.md) for the merged
implementation decisions and fresh verification; historical checkboxes are not
proof that the merged version passed on a particular backend or toolchain.

## Goal

Provide one portable rendering API for OpenGL, Vulkan, and D3D12. Renderer
code describes work and resource usage without native graphics types; backends
translate that description into synchronization and submission operations.

## Combined milestones requested 2026-09-12

- [~] Completion follow-up: graph-owned command recording/submission and portable
      framebuffer attachments; graphics/compute dependency submission with early
      Vulkan semaphore waits and safe semaphore retirement; shared-family Vulkan
      allocations where native queues differ. Keep whole-resource conservative
      barriers and explicitly reject unsupported queue/state combinations.
      Acceptance: an example actually records graphics -> compute -> graphics
      via the graph, all three backends build/run, final presentation waits for
      graph work, repeated frames and shutdown have clean validation. Document
      supported attachment operations and remaining precision limitations.
      Extend DX12 memory isolation with replacement on/off controls; distinguish
      live engine resources from native-driver allocations before changing policy.
      Runtime discovery: DX12 reflection rejects D3D_SIT_UAV_RWBYTEADDRESS used
      by the shared raw storage-buffer example. Map it to the existing raw UAV
      storage-buffer implementation and throw explicitly for unsupported types
      in Release too. Acceptance: both graphics and compute reflection/binding
      succeed and the GPU graph completes normally with the debug layer enabled.
      Add opt-in DX12 pipeline lifetime counters for replacement-on/off memory
      isolation; counts must remain bounded instead of tracking total frames.
      Follow-up: D3D12 material construction creates a null UAV with both
      resource and descriptor null before SetResource. This removes the device.
      Supply valid typed null descriptors while resources are not yet bound;
      rerun the storage-buffer graph to verify construction and later binding.
      Add a synchronous diagnostic StorageBuffer::ReadData operation across all
      APIs, with bounds checks and explicit completion before staging readback.
      The example checks the actual compute-written values at frame 119 once
      (odd frame, different from the initial CPU value, so a missing dispatch
      cannot accidentally pass);
      no per-frame readback/wait is introduced. Acceptance: numeric result check
      succeeds in Debug/Release on all three APIs, not just a successful launch.
      Harden graph target read-before-write/state validation, stale builders and
      misuse of CPU Execute with recording passes before GPU allocation/submission.
      Graph attachment review also found D3D12 offscreen Begin indexing a missing
      depth descriptor and binding/clearing only the first color target. Make
      depth optional and bind/clear the declared color-target count; verify the
      existing color/depth example, keep untested MRT/MSAA cases explicit.

      Intermediate verification: Debug build passed. OpenGL/Vulkan GPU graph
      completed 120 frames normally. After raw-buffer reflection and null-UAV
      fixes D3D12 also completed 120 frames with its debug layer enabled; one
      buffer initial-state warning remains. The first isolated tests lacked
      required input-threshold config keys and timed out before logging; these
      do not count as backend failures. Fixed test configs, no user config edits.
      D3D12 125-second replacement/static comparison both exited normally:
      replacement 54,827 frames, 188608512 -> 210681856 private bytes (10->120s);
      static 56,403 frames, 182513664 -> 204754944 private bytes. At 1,000 total
      pipeline creations only 2 were live. Growth persists without replacement;
      this rules out unbounded live pipeline instances, not all native leaks.
      Readback follow-up: Debug build passed after correcting the D3DX12 helper
      name hiding a barrier union field. OpenGL, Vulkan and D3D12 each completed
      120 frames normally and passed the frame-119 compute readback check. Vulkan
      had no validation errors. D3D12 had no errors, but the buffer-initial-state
      warning remains. OpenGL emitted shader recompilation and one-time readback
      buffer-migration performance warnings. Release verification follows.
      Validation gap: default Vulkan selection uses family 0 for graphics,
      compute and transfer. Add opt-in GENGINE_VULKAN_DEDICATED_QUEUES=1 selection
      of available non-graphics compute/transfer families. Acceptance: logs prove
      distinct families when available and the same GPU readback/shutdown tests
      pass; a shared-family run must not be reported as independent-queue proof.
      Final safety review: reject invalid dependency command buffers and check
      newly added Vulkan semaphore/map calls outside assertions so Release does
      not continue with invalid handles after failure.
      D3D12 debug-layer-off evidence: static 125-second run completed 196,839
      frames, normal exit, readback passed; private bytes 167931904 at 10s and
      167751680 at 60/70/80/90/100/110/120s. Replacement-enabled run completed
      133,568 frames normally; 164360192 at 10s, 164823040 at 30s, 164675584 at
      60/70/80/90/100/110/120s. The observed growth is confined to the enabled
      debug-layer path in these comparisons, not established as an engine leak.
      Keep debug-layer growth separately open; do not disable validation to hide
      errors or add per-frame device waits. Shader readback passed in both runs.
      OpenGL/Vulkan replacement runs lasted 45s with readback and normal exit:
      113,259 / 28,920 frames. OpenGL private bytes 171966464 at 10/20/30s and
      171892736 at 40s; Vulkan 403472384 / 403746816 / 403816448 / 404041728.

- [~] Reliability: repair Release definitions in Premake, add an opt-in bounded
      example run that exits through Application::Close, and test available APIs
      separately for startup/replacement/shutdown and memory. Track any discovered
      lifecycle failures before fixing them; do not claim long-run proof from smoke tests.
      Inspection: D3D12 debug logger thread is started but never stopped; join it
      during context shutdown before releasing its info queue/device.
      Found undefined behavior in OpenGL/Vulkan Material::GetShader: a reference
      to a temporary shared_ptr is returned. Change the common accessor to return
      shared ownership by value across all backends; recheck optimized runs.
      Add a seconds-based example limit for repeated-frame memory sampling with
      normal shutdown, independent of backend frame rate.
- [~] FrameGraph: introduce explicit resource versions and a pass builder with
      regression checks; complete stage/access, attachment and queue semantics
      without silently treating unsupported operations as no-ops.
      Versions, producer/reader/overwrite dependencies, stale-version rejection,
      declaration builders and same-state shader-write barriers are implemented.
      GPU recording builders, graph-created framebuffer targets, dependency
      submission and final presentation joins are implemented in the follow-up.
      Vulkan uses concurrent-family allocations instead of exclusive ownership
      transfers. Detailed stage/subresource scopes and dedicated transfer graph
      passes remain unsupported; the current graph uses whole-resource barriers.
- [~] Device creation: move remaining resource factories behind RenderDevice,
      preserving static compatibility adapters; exercise device creation in the
      example. Keep backend module extraction and pooling outside this request.
      All shader/material/sampler/graphics-compute pipeline/storage/cube/array/
      combined-sampler factories now delegate native construction to RenderDevice.
      Static shader and sampler adapters preserve their runtime caches. Device
      factory calls are uncached; simultaneous active devices remain unsupported.

Acceptance: affected Debug/Release builds, examples, updated API docs and per-API
verification evidence. Unfinished portions remain open rather than hidden by
partial implementations.

Verification update: Debug and Release engine/example builds passed on 2026-09-12.
All three backends passed the final startup regressions and 120-frame Release
normal-exit check (exit code 0). Debug timed runs lasted 20 seconds and exited
normally: OpenGL 49,924 frames, Vulkan 18,780, D3D12 12,172. Vulkan reported no
validation errors; D3D12 debug logging stopped cleanly with no logged errors.
OpenGL emitted only the known shader-recompilation performance warnings.
Private bytes at approximately 5/10/15/20 seconds:
OpenGL 165945344 / 165945344 / 165945344 / 165945344;
Vulkan 398544896 / 398680064 / 398680064 / 398680064;
D3D12 186056704 / 186441728 / 187330560 / 187949056.
D3D12 growth needs longer observation; this is not proof of bounded memory.
Follow-up D3D12-only Debug run with the debug layer enabled lasted 65 seconds,
completed 66,378 frames, and exited normally with code 0 and no logged errors.
Private bytes at 10/20/30/40/50/60 seconds were 184799232 / 185425920 /
186163200 / 186425344 / 187092992 / 187826176. Growth persisted, so memory
stability remains open. Reproduce with GENGINE_EXAMPLE_SECONDS=65 and frame
limit zero; compare longer replacement-on/off runs and allocation ownership
before attributing growth to an engine leak or driver caching. Acceptance:
establish a plateau or identify and fix unreclaimed allocations, then rerun
normal shutdown with debug validation. Do not mask growth with per-frame waits.
Visible output, interactive resize, independent compute/transfer retirement,
and real GPU compute-chain barrier correctness remain unverified in this batch.

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
- [~] Complete Vulkan deferred deletion for all replaceable GPU resources.
        Historical local-branch evidence follows. The remote follow-up below
        completes render-pass retirement; merged-version verification is tracked
        in `CrossComputerIntegration.md`, not inferred from either parent.
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
- [x] Remote-branch Vulkan retirement follow-up (historical evidence).
      Submission-serial retirement is implemented and graphics/compute pipeline
      replacements, core buffer/image/sampler objects, framebuffers, material
      descriptor sets, shader modules/layouts, and descriptor pools use it.
      Source audit after the render-pass follow-up found no remaining
      replaceable Vulkan resource that is destroyed without either
      submission-serial retirement or an explicit completion wait. One-shot
      staging buffers are released after their single-time submission fence
      waits; context-owned command pools, sync objects, and swapchain state are
      released after `WaitForIdle`; presentation fences are released after the
      presenter's idle wait.
      Bounded follow-up completed: `VulkanRenderPass` handles are retired through the same
      submission-serial path, initialize the handle to `VK_NULL_HANDLE`, and
      exercise `FrameBuffer::SetRenderPassOperation` replacement in
      FrameGraphTriangle alongside the existing buffer/pipeline replacement.
      Implemented: `VulkanFrameBuffer::SetRenderPassOperation` now creates a
      backend render pass directly instead of reusing the static cache, so
      each operation replacement actually retires the old native handle.
      Verified: Release/Vulkan builds, renders through repeated render-pass
      replacement, accepts `WM_CLOSE`, exits with code 0, and writes no
      stderr.
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
      used the `universal1.5` SPIR-V environment without Vulkan reflection
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
      Startup regression scenarios passed in Debug and Release on all three
      backends on 2026-09-12, including normal application shutdown.
      Verified: FrameGraphTriangle's startup checks executed on Release
      D3D12, Vulkan, and OpenGL during the backend retirement and rendering
      checks recorded above. Each run reached rendering after rejecting the
      invalid transient graphs and detecting the inferred dependency cycle.

- [~] Add typed image/buffer descriptors and transient resource creation.
      `RenderGraph` can now create transient 2D textures, storage buffers, and
      storage images from portable descriptors. Pooling and aliasing remain
      intentionally deferred until lifetime analysis is in place.
- [x] Track resource versions, reads, writes, first/last use, and final state.
      First/last use and final state are now recorded for every compiled
      resource. Versioned handles and explicit read/write propagation now reject
      stale versions and branching writes; startup tests cover producer ordering
      and overwrite-after-reader dependencies. This does not enable aliasing.
- [~] Add stage/access intent, subresource ranges, queue ownership, and
      per-backend barrier compilation.
      Graphics/compute queue dependencies are now submitted by ExecuteGpu;
      D3D12 uses command-generation fences, Vulkan uses per-edge semaphores and
      concurrent-family resources. Whole-resource storage barriers are tested.
      Fine-grained stage/subresource scopes and dedicated transfer builders remain.
- [ ] Add resource pooling and aliasing only after lifetime tracking is tested.
- [x] Expose pass builders to renderer features so off-screen work is declared
      instead of hidden in `Layer::OnRender`.
      GPU builders own offscreen attachment recording and submission. The example
      uses the graphics/compute builders, with numeric GPU result verification
      in Debug on all three backends; Release results are recorded above.

Acceptance: a graphics -> compute -> graphics chain can run without any
backend-specific barrier code in a layer or renderer feature.

## Phase 4 — device-owned creation and modules

- [x] Move Shader, Material, Sampler, Pipeline, texture, and compute factories
      under `RenderDevice`; remove central API switches from common sources.
      All three native implementations build in Debug/Release. The example
      exercises shader/material/graphics-pipeline/vertex/target creation, not
      every cube/array/storage-image factory variant. The GPU graph example now
      also exercises raw storage-buffer and compute-pipeline creation/readback.
- [ ] Build OpenGL, Vulkan, and D3D12 as separate backend modules.
- [ ] Replace the Vulkan-only subpass public contract with portable graph pass
      attachments; retain a legacy adapter only where needed.
- [ ] Make capabilities data-driven from queried device features, not hardcoded
      booleans or conservative constants.

Acceptance: adding a backend requires a backend module and registration only;
it does not require editing every common resource factory.

## Verification matrix

- [x] Fix Release build definitions: PhysX requires exactly one of `NDEBUG`
      and `_DEBUG`; the current Release configuration fails with C1189.
      Re-run the assertion-disabled FrameGraphTriangle startup check afterward.
      Fixed in workspace Premake for Release/Dist and regenerated projects.
      Release engine/example build passed; OpenGL, Vulkan and D3D12 each completed
      120 frames with normal exit code 0 on 2026-09-12, including versioned graph
      startup checks. Final overwrite and same-state dependency regressions were
      rebuilt and rerun successfully on all three backends afterward.
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

The following matrix is historical remote-branch evidence, not merged-version
verification. Current integration results are tracked in `CrossComputerIntegration.md`.

| Check | OpenGL | Vulkan | D3D12 |
| --- | --- | --- | --- |
| Acquire / present | passed | passed | passed |
| `float3` vertex input | passed | passed | passed |
| Replacement survival (not memory telemetry) | passed | passed | passed |
| Captured runtime output | passed | passed | passed |

OpenGL evidence: 99,467 frames, 3,315 replacements, `WM_CLOSE`, exit code 0,
GL error 0, and empty stderr. Vulkan and D3D12 evidence is recorded above.
Memory stability here means surviving the exercised replacement path without
a crash; it is not a heap-growth telemetry measurement.

Current local build note: automated compilation must run from a clean Visual
Studio developer environment.  The current host process exports both `PATH`
and `Path`, which causes MSBuild to fail before `CL.exe` receives any source.
