      transfer builders, and pooling/aliasing remain separate open work.
      Next milestone 2026-10-03: replace the empty array-layer transition
      callbacks with an actual backend-neutral texture copy on the graphics
      queue. Add a synchronous diagnostic readback for one RGBA8 array layer;
      copy a known pixel into layer 0 and prove numerically that layer 0 changed
      while layer 1 kept its distinct initial value. Keep the copy inside a
      graph pass without a render target, declare source/destination ranges,
      and restore both resources to shader-readable state across frames.
      Acceptance: invalid dimensions/layers/formats are rejected, Debug and
      Release engine/example builds pass, and OpenGL, D3D12, Vulkan shared,
      and Vulkan dedicated runs pass the frame-119 pixel check and normal
      shutdown. Record Vulkan validation output and D3D12 debug output.
      Leave dedicated transfer-queue builders and exclusive ownership transfer
      for a later milestone, then revisit pooling only after visible/resize and
      lifetime checks.
      Implemented in the working tree: targetless graphics copy pass, a common
      2D-to-array-layer copy API with backend validation, RGBA8 layer diagnostic
      readback, and a real two-layer pixel regression in FrameGraphTriangle.
      The example checks invalid layer, dimensions, and format during recording
      before the valid copy. Debug engine and example builds passed; an initial
      example compile error from the Texture2D SetData overload was corrected.
      With the negative-copy cases, Debug OpenGL, D3D12 debug layer, Vulkan
      shared, and Vulkan dedicated all completed 120 frames, passed compute and
      array-pixel frame-119 readbacks, and exited 0. Vulkan dedicated selected
      graphics=0, compute=2, transfer=5; neither Vulkan run logged a validation
      error, and D3D12 logged no debug error or warning. VS2022 Release x64
      engine/example builds passed; Release OpenGL, D3D12, Vulkan shared, and
      Vulkan dedicated each completed 120 frames, passed both readbacks, and
      exited 0. Release logs likewise showed no D3D12 debug error/warning or
      Vulkan validation error. The bounded runs do not verify manual visible
      output, interactive resize, or longer-term memory stability; keep those
      lifecycle checks open. Dedicated transfer work and ownership transfer
      also remain separate follow-ups.
      Transfer-pass milestone (completed, 2026-10-18): add a portable
      graph copy builder that records on the D3D12 copy queue and Vulkan
      transfer queue, with an explicit single-context graphics fallback on
      OpenGL. Move required state/layout transitions to a graphics preparatory
      submission before dedicated transfer work, then join transfer results
      with graph consumers and presentation. Permit the existing validated
      2D-to-array-layer copy operation on transfer command buffers. Acceptance:
      a real example copy uses this builder, invalid copy inputs still fail,
      Debug/Release engine and example build, and OpenGL, D3D12 Debug Layer,
      Vulkan shared/dedicated runs pass both pixel readbacks without new
      validation errors. Vulkan graph resources currently use concurrent
      sharing across distinct families; exclusive-sharing ownership transfer
      remains a separate explicit TODO, not implied by this milestone.
      First D3D12 Debug Layer run exposed a queue-type state rule: resources
      prepared on DIRECT in COPY_SOURCE/COPY_DEST trigger errors when used
      by COPY. Correct the backend boundary so D3D12 transitions them to
      COMMON before the copy queue, which promotes/decays copy states; keep
      Vulkan copy layouts on the graphics preparatory submission. The run's
      process exit 0 is not accepted as validation success.
      The COMMON handoff removed per-frame COPY errors. A 118-frame Debug
      Layer run logged zero issues; at frame-119 diagnostic readback, one
      array-layer state mismatch remained because the native COPY queue
      decayed the array resource to COMMON while the D3D12 texture tracker
      still held GENERIC_READ for an untouched layer. Update tracked texture
      states after copy-queue recording and recheck the readback path.
      D3D12 120-frame rerun now passes both readbacks with zero Debug Layer
      issues. OpenGL fallback also passes. First Vulkan shared run exited with
      `0xC0000094` before frame-119 checks: its context allocated native
      transfer command buffers but never wrapped them in the reusable
      `m_TransferCommandBuffers` list, so retrieval divided by zero. Populate
      the wrappers, then rerun both Vulkan queue modes.
      Final verification: VS2022 Debug and Release engine/FrameGraphTriangle
      builds passed. After the final transfer-preparation change, OpenGL,
      D3D12, Vulkan shared, and Vulkan dedicated modes each completed 120
      frames in both configurations, passed compute and array-layer GPU
      readbacks, and exited 0 with empty stderr and no captured validation/
      debug errors. Vulkan dedicated selected graphics=0, compute=2,
      transfer=5. D3D12 Debug/Release synthetic-resize runs also logged
      both 960x540 and 1280x720 recreations and passed readbacks with no
      Debug Layer issues. The example retains invalid layer/size/format
      rejection. This validates the existing texture-to-array copy path,
      not arbitrary resource types, visible output, long-run memory, or
      Vulkan exclusive queue-family ownership.
      Repeated-frame follow-up: Debug D3D12 and Vulkan dedicated each ran
      10,000 frames, passed both readbacks, exited normally, and logged no
      D3D12 Debug Layer or captured Vulkan validation issues. A 30-second
      Vulkan dedicated run completed 21,139 frames; process private bytes
      sampled at 10/20/30 seconds were 414,900,224 / 414,900,224 /
      414,826,496, with normal shutdown and zero validation messages. This
      bounded sample does not prove indefinite memory stability; the prior
      late Vulkan increase and D3D12 validation-layer CPU growth remain
      separate open investigations.
      Noninteractive 120-second Debug replacement checks on 2026-10-11 are
      recorded in [CrossComputerIntegration.md](../CrossComputerIntegration.md).
      OpenGL was near-flat, Vulkan dedicated queues rose late by about 4.7 MB,
      and D3D12 with its debug layer rose about 17.8 MB while a layer-off
      control stayed near-flat. A no-replacement D3D12 debug-layer control
