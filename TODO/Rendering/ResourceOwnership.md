- [ ] Add Vulkan exclusive-sharing queue-family ownership transfer as a
      separate feature. Current graph-created buffers/images use concurrent
      sharing when graphics/compute/transfer families differ, so the new
      dedicated transfer pass needs semaphores and layout transitions but
      not ownership release/acquire. Acceptance for a future exclusive mode:
      track family ownership per resource/range; record release and acquire
      barriers on the correct queues around graph dependencies; reject
      unsupported sharing modes explicitly; exercise graphics -> transfer ->
      graphics on separate families with validation and pixel readback.
      Current milestone (2026-10-19): implement an opt-in exclusive image
      path for graph transfer copies while preserving concurrent sharing by
      default. Acceptance: graphics-prepared copy ranges release to transfer,
      transfer acquires before copying and releases afterward, and the next
      graphics reader acquires them. Validate matching barriers and semaphores
      with the existing array-layer readback on distinct queue families.
      Buffers and general graphics/compute handoffs remain open until their
      own ownership and lifetime paths are covered.
      Implemented: `GENGINE_VULKAN_EXCLUSIVE_TRANSFER_IMAGES=1` creates
      Vulkan Texture2D/Texture2DArray images with exclusive sharing. The graph
      tracks each mip/layer's graphics -> transfer -> graphics ownership,
      records matching release/acquire barriers around existing semaphore
      dependencies, and rejects unsupported exclusive-image compute access.
      Normal Vulkan image sharing remains concurrent. Source uploads and
      array initialization are graphics-queue work before graph execution.
      Verification (2026-10-19): VS2022 Debug and Release engine/example
      builds passed. OpenGL, D3D12, Vulkan shared (normal and opt-in), and
      Vulkan dedicated (normal and opt-in) each completed 120 frames in both
      configurations; compute and array-layer readbacks passed, exit was 0,
      stderr empty, and no captured validation/Debug Layer issues appeared.
      Dedicated Vulkan selected graphics=0, compute=2, transfer=5. Final
      Debug dedicated opt-in run completed 10,000 frames with both readbacks,
      normal shutdown, and zero captured validation issues.
      A 32-second Debug dedicated opt-in run completed 19,603 frames; private
      bytes at 10/20/30 seconds were 387,371,008 / 387,371,008 / 387,330,048,
      with both readbacks, normal shutdown, and no captured validation issue.
      This bounded sample does not prove indefinite memory stability.
      These runs validate the example's texture-to-array ranges only, not
      arbitrary exclusive resources, external queue users, visible output,
      or indefinite memory
      stability. Keep the overall TODO open for exclusive buffers and general
      compute/graphics ownership, with further examples and validation.
      Next milestone (2026-10-19): add opt-in exclusive Vulkan storage buffers
      and graph-managed graphics -> compute -> graphics ownership handoff.
      Scope includes graphics-side preparation for an initial compute use,
      matching buffer release/acquire barriers and semaphore edges, repeated
      frames, readback, and explicit rejection of unsupported queue/resource
      combinations before submission. Acceptance: Debug/Release engine and
      FrameGraphTriangle builds; dedicated graphics/compute families pass the
      example's numeric compute readback with Vulkan validation enabled;
      default concurrent mode and OpenGL/D3D12 behavior remain unchanged.
      Check replacement, shutdown, and bounded repeated-frame memory behavior.
      Extend FrameGraphTriangle with a second buffer whose first graph access
      is compute, followed by a real graphics draw and numeric readback; keep
      the existing graphics-first buffer as the graphics -> compute regression.
      Exclusive compute images and arbitrary external queue users remain
      separate unless this milestone explicitly implements and tests them.
      Implemented: `GENGINE_VULKAN_EXCLUSIVE_COMPUTE_BUFFERS=1` selects
      exclusive sharing for Vulkan storage buffers only. Each compute graph
      pass that accesses one receives a graphics preparatory submission;
      graphics release, compute acquire/dispatch/release, and later graphics
      acquire are tracked per buffer. Transfers of exclusive compute buffers
      and compute use of exclusive transfer images fail in graph preflight.
      Buffer CPU upload/readback reject non-graphics ownership. The example
      now tests both graphics-first and compute-first buffers with real draws
      and separate numeric readbacks.
      Verification (2026-10-19): VS2022 Debug/Release engine and example
      builds passed. OpenGL, D3D12, Vulkan shared (normal and both opt-ins),
      and Vulkan dedicated (normal, buffer-only, and both opt-ins) each ran
      120 frames in both configurations. All three GPU readbacks passed,
      processes exited 0 with empty stderr, and no captured D3D12 Debug
      Layer/Vulkan validation issues. Dedicated Vulkan selected graphics=0,
      compute=2, transfer=5. Debug dedicated with both opt-ins ran 10,000
      frames, passed all readbacks, and shut down without captured issues.
      A 32-second Debug dedicated run with both opt-ins and replacement
      completed 20,244 frames; private bytes at 10/20/30 seconds were
      388,677,632 / 390,815,744 / 393,048,064. A concurrent-sharing control
      with replacement also rose (387,911,680 / 390,205,440 / 392,130,560),
      so this sample does not isolate ownership as the cause. With replacement
      disabled, concurrent control was 413,261,824 / 413,261,824 / 413,614,080
      and both-opt-in run was 412,418,048 / 412,418,048 / 412,385,280 at
      10/20/30 seconds; both shut down normally without validation messages.
      After the final shared-family preparation check, Debug/Release rebuilt
      successfully and dedicated-family Debug/Release plus shared-family
      Debug both-opt-in 120-frame regressions passed all three readbacks.
      Keep the replacement-linked growth under investigation; these bounded
      runs do not prove indefinite memory stability. Exclusive compute images,
      external queue users, and unsupported-combination negative runtime tests
      remain open beyond this milestone.
      Replacement-growth follow-up acceptance: repeat matched dedicated
      Debug runs with validation on/off and replacement on/off, compare live
      pipeline and retirement counts or an allocation trace, identify the
      allocation owner of the ~4 MB/20 s rise, then rerun the same sample
      after any fix. Do not attribute it to the ownership path from these
      measurements alone.
- [ ] Add resource pooling and aliasing only after lifetime tracking is tested.
      Prerequisite: complete the manual visible-output/resize/minimize checks
      recorded in [CrossComputerIntegration.md](../CrossComputerIntegration.md),
      then add regression coverage for fine-grained resource lifetimes. Do not
      start pooling until attachment semantics and capability reporting make
      alias ownership explicit.
- [x] Expose pass builders to renderer features so off-screen work is declared
      instead of hidden in `Layer::OnRender`.
      GPU builders own offscreen attachment recording and submission. The example
      uses the graphics/compute builders, with numeric GPU result verification
      in Debug on all three backends; Release results are recorded above.

Acceptance: a graphics -> compute -> graphics chain can run without any
backend-specific barrier code in a layer or renderer feature.
