## Phase 3 — real frame graph

- [x] Close the dedicated transfer-builder milestone against current sources
      (2026-10-31, user authorized). Scope: audit the existing
      `RenderGraph::BuildTransferCopyPass`, state/queue validation, backend
      preparation/submission, and FrameGraphTriangle coverage; correct stale
      tracking/docs; and reverify the path after the later backend-module and
      graph changes. Do not add a new public API or expand transfer resource
      types in this milestone. Acceptance: current CMake Debug/Release
      FrameGraphTriangle builds pass; OpenGL, Vulkan, and D3D12 bounded runs
      pass the existing array-layer/compute readbacks and shut down normally;
      Vulkan dedicated queues are checked when available; documentation and
      this TODO distinguish implemented transfer support from remaining
      fine-grained scheduling and broader ownership work.
      Audit result: `BuildTransferCopyPass` already exists and is used by the
      live FrameGraphTriangle graphics -> transfer -> graphics texture-to-array
      copy. It validates transfer-stage CopySource/CopyDestination declarations,
      acquires a graphics preparation command for D3D12/Vulkan dedicated copy
      queues, submits the copy command, and joins consumers and presentation.
      No new public API or transfer resource type was needed in this milestone.
      Verification: current-source CMake Debug and Release
      FrameGraphTriangle builds passed. OpenGL, Vulkan, and D3D12 completed
      120-frame runs in both configurations; every run passed the compute,
      compute-first, and texture-array readbacks and exited through normal
      shutdown. Vulkan dedicated-queue Debug/Release runs also passed with
      graphics=0, compute=2, transfer=4. D3D12 runs used the debug layer and
      reported no debug-layer issues. `Config.ini` was restored to GraphicsAPI 3.
      Remaining: fine-grained independent scheduling, broader transfer resource
      types, arbitrary external queue users, interactive behavior, and long-run
      memory remain outside this completed transfer-builder milestone.

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
      Graphics/compute/transfer queue dependencies are now submitted by
      ExecuteGpu; D3D12 uses command-generation fences, Vulkan uses per-edge
      semaphores, and the dedicated transfer-copy builder is implemented.
      Fine-grained stage/subresource scopes and broader queue ownership remain.
      Current milestone 2026-09-21: add portable pipeline-stage and read/write
      intent to RenderGraph access declarations, carry it through compiled
      transitions, and make each backend preserve the synchronization semantics
      (with conservative legacy barriers where native stage scoping is unavailable).
      Acceptance: the existing GPU graph explicitly declares graphics and compute
      storage usage; a focused declaration regression confirms the intent reaches
      transition compilation; Debug build and the three-backend readback scenario
      are rechecked. Subresource ranges remain open; transfer-copy builders are
      implemented and verified separately above.
      Verification 2026-09-21: static diff review and `git diff --check` passed.
      The isolated VS2022 Debug example build reached compilation after removing
      the host PATH/Path collision, but the shared Debug intermediate directory
      then reported C1041 for `vc143.pdb`. No source diagnostic was emitted and
      no compiler process remained afterward. Rebuild from an uncontended or
      separately configured intermediate directory before claiming build or
      backend-runtime verification.
      Follow-up 2026-09-27: a serial VS2022 Debug build used a dedicated
      `build-verify` intermediate/output directory after clearing the host
      PATH/Path collision. It completed without leaving compiler processes but
      did not produce `GEngine.dll`; the build host did not preserve its final
      diagnostic. The temporary directory was removed. Build and runtime
      verification therefore remain open; do not use this attempt as source
      success or failure evidence.
      Next milestone 2026-09-27: introduce a portable subresource-range
      declaration with validation. Until native per-range barriers are present
      on all backends, GPU execution must reject a non-whole range explicitly;
      it must never silently widen it to a whole-resource barrier.
      Implemented 2026-09-27: `SubresourceRange` defaults to the complete
      resource, rejects empty declarations, and is retained in compiled graph
      transitions. The FrameGraphTriangle startup regression checks empty-range
      rejection. Non-whole GPU ranges fail before command recording. Per-range
      native barrier compilation and GPU runtime verification remain open.
      Full follow-up authorized 2026-09-27: extend common texture metadata and
      graph imports for mip/layer bounds; thread valid ranges through the
      graphics transition API; compile Vulkan image ranges and D3D12 subresource
      transitions, and document OpenGL's conservative global memory-barrier
      semantics. Add range validation regression coverage. Acceptance: invalid
      ranges fail before recording; valid texture-array layer ranges reach each
      backend without silent whole-resource fallback; build and available backend
      runtime evidence are recorded separately.
      Progress: the common `GraphicsSubresourceRange` and range-aware transition
      overload now exist. GPU execution remains explicitly blocked for non-whole
      ranges until all backend implementations consume that overload.
      Next bounded step 2026-09-29: graph resources carry explicit mip/layer
      metadata, transient descriptors declare their subresource counts, and
      compilation resolves and validates ranges before transient allocation.
      Acceptance: imported textures and storage images expose actual counts;
      texture arrays expose their layer count; `All` counts resolve against
      metadata; empty, out-of-bounds, overflowing, and unsupported non-whole
      ranges fail before allocation or recording. Native per-range barriers and
      full three-backend runtime verification remain outside this step.
      Implemented 2026-09-29: `GraphicsResource` exposes subresource metadata,
      textures and texture arrays report their actual mip/layer counts, storage
      images report one mip/layer, graph imports retain that metadata, and
      transient texture/array/storage-image descriptors declare their counts.
      Compilation resolves `All` and rejects empty, overflowing, out-of-bounds,
      zero-count, and non-whole unsupported ranges before transient allocation.
      The range-aware whole-resource GPU check uses resolved metadata.
      FrameGraphTriangle checks the negative paths plus valid two-mip, imported
      two-mip, transient two-layer-array, and imported two-layer-array ranges.
      Verification: isolated VS2026 Debug and Release builds of GEngine and
      FrameGraphTriangle passed with MSBuild 18.10.1 / MSVC 14.50.35717,
      `/m:1 /nr:false`, warnings but zero errors. The final Debug and Release
      code, including presentation metadata and the transient-array regression,
      was rebuilt after all diagnostic-log removal. OpenGL 4.6 (NVIDIA driver
      610.88) completed 120 frames, passed frame-119 compute readback, and
      exited with code 0 in both configurations.
      Backend runtime: OpenGL 4.6, D3D12 feature level 12.2, and Vulkan 1.4.341
      shared queues each completed 120 frames, passed frame-119 compute
      readback, and exited with code 0 in both Debug and Release. Vulkan
      dedicated queues (graphics=0, compute=2, transfer=4) also passed in both
      configurations. An initial D3D12/Vulkan pre-first-frame stop was traced
      to presentation wrappers returning no subresource metadata; exposing
      their one-mip/one-layer swap-chain images fixed the import path. Vulkan
      validation layers were unavailable on this host, so those runs had no
      validation-layer coverage.
      Next bounded step 2026-09-29: range-aware dependency analysis and
      compiled transition state. The common graph must compare declared access
      ranges, retain exact ranges in transitions, and track state per touched
      range instead of one whole-resource state. Because no backend yet tracks
      or synchronizes independent subresources, disjoint ranges must remain
      conservatively ordered; do not enable independent scheduling. Native
      per-range barriers remain outside this step.
      Implemented 2026-09-29: dependency inference compares declared ranges
      while retaining conservative ordering for disjoint accesses, compiled
      transition state is split and tracked by touched mip/layer rectangles,
      and transition emission uses only the overlapping range fragments.
      `UsageTransitionCallback` now receives the compiled range, and
      `RenderSystem` forwards it through the range-aware transition overload.
      FrameGraphTriangle checks exact transition ranges, untouched-mip state
      isolation, and the expected conservative disjoint-range cycle.
      Verification: isolated VS2026 Debug and Release builds passed with
      MSBuild 18.10.1 / MSVC 14.50.35717, `/m:1 /nr:false`, warnings but zero
      errors. OpenGL 4.6, D3D12 12.2, Vulkan 1.4.341 shared queues, and Vulkan
      dedicated queues (graphics=0, compute=2, transfer=4) each completed 120
      frames, passed frame-119 compute readback, and exited with code 0 in both
      Debug and Release. Vulkan validation layers were unavailable on this host.
      Implementation progress 2026-10-03: D3D12 tracks texture mip and array
      layer states and emits transitions for selected subresources; upload paths
      keep array-layer state coherent. Vulkan tracks layout per 2D texture mip
      and array mip/layer, and emits ranged image barriers. Legacy full-image
      layout changes walk the same state tables. OpenGL accepts valid ranges as
      conservative whole-resource barriers. `ExecuteGpu` prevalidates backend
      range support before command-buffer acquisition/recording. FrameGraphTriangle
      now records a two-layer GPU transition exercise in addition to its compile
      regressions. Debug/Release builds and backend runtime validation remain open;
      do not treat source compilation alone as full acceptance.
      Verification 2026-10-03: `git diff --check` passed. An isolated VS2022
      Debug x64 GEngine and FrameGraphTriangle project builds passed, and the
      full solution build compiled the changed RenderGraph, D3D12, OpenGL, Vulkan,
      and FrameGraphTriangle translation units without source errors. The full
      solution then encountered pre-existing API mismatches in Sandbox and
      GEngine-Editor; those projects are outside this milestone. The Debug
      FrameGraphTriangle executable ran 120 frames using the user's selected
      OpenGL backend, passed the frame-119 compute readback and shut down with
      exit code 0. Vulkan and D3D12 range runtime checks, Release builds, and
      Vulkan validation-layer coverage remain open; preserve the user's local
      OpenGL selection in `Config.ini` when scheduling those checks.
      Follow-up 2026-10-03: isolated Debug D3D12 and Vulkan shared-queue runs
      passed 120 frames and frame-119 readback. Dedicated Vulkan queues also
      completed 120 frames, but validation reported
      `VUID-vkCmdPipelineBarrier-srcStageMask-06461` on each compute submission:
      a graphics-only source stage was recorded on the compute queue. Correct
      the queue-local barrier stage/access scope, then require a dedicated-queue
      run with zero validation errors as the acceptance check. Release builds
      and per-backend runs are still required.
      Resolution 2026-10-03: Vulkan now constrains barrier stages to the
      recording queue. Unsupported prior-queue source stages use top-of-pipe
      with no source access; graph submission semaphores provide the cross-queue
      dependency. The Debug GEngine and FrameGraphTriangle projects rebuilt,
      and Vulkan shared/dedicated 120-frame runs both passed frame-119 readback
      with zero validation errors. The dedicated run selected graphics=0,
      compute=2, transfer=5. VS2022 Release x64 GEngine and FrameGraphTriangle
      builds passed; Release OpenGL 4.6, D3D12 12.2, Vulkan shared, and Vulkan
      dedicated each completed 120 frames and passed frame-119 readback with
      exit code 0. Debug OpenGL and D3D12 had already passed. Runtime testing
      covered barrier recording and normal shutdown, not interactive visible
      output, resize, or long-run memory stability. Queue ownership, dedicated

- [x] Resolve D3D12 debug-layer cross-queue UAV-barrier diagnostics.
      Scope: FrameGraphTriangle D3D12 Debug/Release runs with
      `GENGINE_D3D12_DEBUG_LAYER=1` exit 0 and pass all three readbacks, but the
      debug layer reports that a buffer is still referenced by an in-flight
      UAV barrier on another command queue. Identify the exact producer and
      consumer queues and buffer, then correct cross-queue synchronization or
      resource ownership.
      Acceptance: bounded 120-frame D3D12 runs in Debug and Release pass all
      three graph readbacks with no D3D12 debug-layer errors.
      Active follow-up (2026-10-28, user authorized): reproduce with the current
      FrameGraphTriangle, identify the native buffer and graphics/compute queue
      dependency (including consecutive graph executions), implement a bounded
      synchronization fix without per-frame device-wide waits, and verify Debug/
      Release 120-frame readbacks plus debug-layer output. Preserve the separate
      planned resource-data TODO and local backend configuration. Record focused
      regression, queue lifetime/allocation evidence and other-backend coverage.
      Reproduction: current Debug ran 120 frames, passed all three readbacks but
      emitted 5 UAV errors. With native object names, a second baseline emitted
      11 errors identifying StorageBuffer#2 (ComputeFirstColor): GraphicsQueue
      still referenced it when the next frame's ComputeQueue began its barrier.
      Graph edges covered one execution but omitted this inter-execution seam.
      Implemented: UAV buffer/image-owned last submitted fence/value/queue token;
      command-local deduplicated owner/use records populated by barriers and
      material binding; a different queue waits before execution and updates the
      token only after its submission signal. Discarded recording cannot publish
      an unsignaled completion. Safe allocator reset clears uses after completion;
      native fence references create no C++ command/resource owner cycle. CPU
      storage uploads/readbacks also respect a preceding other-queue submission.
      No new per-frame native fence, CPU per-frame device wait, or common API.
      Release opt-in validation is now real (previously GE_DEBUG guarded it out).
      Bounded info-queue polling retains an append-only cursor and drains after
      GPU completion, avoiding messages lost by read/clear races; UAV errors are
      not filtered. Added native queue/buffer names and an example discarded-
      graphics recording before the compute-first pass.
      Verification (2026-10-28): CMake Debug and Release engine/example builds
      passed with existing warnings. Tests/D3D12QueueRegression.py passed both
      configurations: replacement and no-replacement 120-frame runs, discarded
      recording plus two synthetic resizes, and 15-second stress; every case
      passed all three readbacks, exited 0 and logged zero D3D12 warnings/errors
      with the logger explicitly active. A further 10-second Release trace kept
      pending submissions bounded (1-5 in final samples), deferred releases 0,
      stored/discarded messages 0. Debug and Release OpenGL/Vulkan/D3D12 resource
      matrices each passed cold/warm 120-frame readbacks independently.
      Limits: storage-image bookkeeping compiled but has no dedicated GPU image
      workload here; manual OS resize/visible pixels, Premake and clean-machine
      builds are not claimed. A pre-existing layer-on kernel-handle growth also
      reproduced on the old binary and remains open in Integration/D3D12Memory.md;
      this milestone establishes synchronization, not complete memory attribution.
      Completed follow-up (2026-10-29, user authorized): finish the declaration-to-
      barrier synchronization for resource state, read/write intent, and pipeline
      stage. Reject write-only states declared as reads, read-only states declared
      as writes, presentation declarations with an explicit shader/transfer stage,
      and state/stage combinations the public model cannot express. Validate the
      declared stage against graphics/compute/transfer pass queues before
      transient allocation or command recording, and make Vulkan derive access
      masks only from declarations that can legally map to native access scopes.
      Acceptance: focused positive/negative regressions cover declaration
      validation and queue-stage rejection; Debug/Release builds and available
      backend readback runs are recorded separately. Subresource independent
      scheduling and pooling remain outside this bounded follow-up.
      Implemented: `RenderGraph` validates state/access and state/stage
      compatibility when an access is declared, then validates the declared stage
      against the pass queue during compilation. FrameGraphTriangle adds positive
      intent and three negative startup checks (write-state/read intent, shader
      state/transfer stage, and compute stage on a graphics pass). Vulkan no
      longer substitutes an incompatible access fallback and removes compute-shader
      scope from graphics command barriers.
      Verification: CMake Debug and Release GEngine/FrameGraphTriangle builds
      passed with existing warnings and zero errors. D3D12 Debug Layer-on
      Debug/Release, OpenGL Debug/Release, Vulkan shared Debug/Release, and Vulkan
      dedicated-queue Debug/Release each completed 120 frames, passed all three
      frame-119 readbacks, and exited 0 with no captured D3D12 or Vulkan
      diagnostics. The original D3D12 `Config.ini` selection was restored. Vulkan
      validation layers were unavailable on this host, so those runs did not
      provide validation-layer coverage.
