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
