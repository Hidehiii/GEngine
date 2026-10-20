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
      Later D3D12 state follow-up (2026-10-18): the storage buffer now starts
      in COMMON and completes a one-time COMMON-to-UAV transition. Debug
      Layer-on and Release 120-frame runs passed both GPU readbacks without
      the prior initial-state warning. The separate validation-enabled CPU
      private-byte growth persists; see `TODO/CrossComputerIntegration.md`.
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
