## Phase 4 — device-owned creation and modules

- [x] Move Shader, Material, Sampler, Pipeline, texture, and compute factories
      under `RenderDevice`; remove central API switches from common sources.
      All three native implementations build in Debug/Release. The example
      exercises shader/material/graphics-pipeline/vertex/target creation, not
      every cube/array/storage-image factory variant. The GPU graph example now
      also exercises raw storage-buffer and compute-pipeline creation/readback.
- [x] Build OpenGL, Vulkan, and D3D12 as separate backend modules.
      Current bounded milestone (2026-10-31, user authorized): keep the public
      render API and runtime backend selection unchanged, but split each
      platform source set and its native link dependencies into named static
      backend modules for both CMake and Premake. The engine links all three
      modules by default so GraphicsAPI selection remains unchanged. Do not
      remove backend headers or introduce runtime dynamic loading in this
      milestone. Acceptance: generated targets visibly separate OpenGL, Vulkan,
      and D3D12; Debug/Release engine plus FrameGraphTriangle builds pass;
      bounded 120-frame runs pass on all three APIs with both readbacks and
      normal shutdown; common sources no longer contain native backend link
      dependencies in build metadata; architecture/API/build documentation and
      this TODO are synchronized. Remaining module-boundary cleanup (private
      backend headers, optional backend build switches, dynamic registration,
      and incremental-build measurements) stays explicitly out of scope.
      Implemented: CMake now globs each `GEngine/src/Platform/{OpenGL,Vulkan,
      D3D12}` tree into a static backend target, removes those sources from the
      common `GEngine` target, and moves each backend's native link dependencies
      to that target. Premake similarly defines the three static backend
      projects, removes backend sources from `GEngine`, and links the modules
      into the shared engine. `WindowsResourcePack.cpp` is excluded from the
      engine PCH so its existing resource-pack regression include guard remains
      valid. Public APIs, backend headers, runtime selection, and default
      all-backend linking are unchanged.
      Verification: CMake Debug and Release GEngine plus FrameGraphTriangle
      builds passed; Premake Debug and Release GEngine plus FrameGraphTriangle
      builds passed after VS2022 regeneration. CMake Debug, CMake Release, and
      Premake Release each completed OpenGL/Vulkan/D3D12 120-frame runs with
      all three GPU readbacks, normal shutdown, and no validation/assert/error
      lines; Premake Debug completed the same matrix before the final source
      cleanup. Logs are `build/backend-split-*.log`; build logs record only
      existing warnings. VS2019/VS2026, optional backend switches, dynamic
      registration, private header movement, and incremental-build measurements
      remain open.
- [~] Replace the Vulkan-only subpass public contract with portable graph pass
      attachments; retain a legacy adapter only where needed.
      Scope: introduce backend-neutral graph attachment descriptors for color,
      depth, load/store operation, sample count, and the attachment accesses a
      graph pass needs. Keep native subpass descriptions behind the backend
      adapters; graph-facing APIs must not require Vulkan stage/access masks.
      Preserve a compatibility adapter for existing `RenderPassSpecification`
      callers while the graph API migrates. Unsupported combinations must fail
      explicitly in Debug and Release rather than becoming backend no-ops.
      Acceptance: FrameGraphTriangle declares the same attachment through the
      portable descriptor on OpenGL, Vulkan, and D3D12; a subpass-enabled
      scenario works on Vulkan while the other backends report unsupported
      semantics without silently changing behavior.
      Implemented 2026-09-21: `RenderGraph::AttachmentSpecification` and
      `SubpassSpecification` provide the graph-facing color/depth/load/store/
      sample/subpass contract without Vulkan masks. Invalid attachment and
      subpass indices are rejected before target allocation. The existing
      `RenderPassSpecification` overload remains the compatibility adapter.
      Capability rejection happens before graph target creation. The example
      uses the portable descriptor and exercises an explicit Vulkan subpass.
      Verification found and fixed Vulkan explicit-subpass assembly indexing an
      empty multisample-reference vector when `Samples == 1`. It also found
      Debug Vulkan requesting an unavailable validation layer and then failing
      `vkCreateInstance`; Vulkan now falls back to no validation layer, and the
      debug messenger handle is initialized to null and destroyed only when
      valid.
      Final VS2026 Debug (MSVC 14.50.35717) and Release (MSVC 14.51.36231)
      engine/example builds passed with warnings but zero errors. Debug and
      Release each ran OpenGL, Vulkan shared queues, Vulkan dedicated queues
      (graphics=0, compute=2, transfer=4), and D3D12 for 120 frames. Every run
      passed frame-119 readback and exited with code 0. D3D12 Debug used the
      debug layer and logged no errors. Vulkan validation layers were not
      installed on this host; Debug Vulkan logged the fallback explicitly and
      therefore had no validation-layer coverage. Full migration away from
      `RenderPassSpecification` and data-driven capability reporting remain
      open.
- [~] Make capabilities data-driven from queried device features, not hardcoded
      booleans or conservative constants.
      Scope: populate `GraphicsCapabilities` from actual backend/device queries,
      extension/feature checks, and queried limits. Keep capability reporting
      separate from API identity; do not infer one backend's support from
      another. Record one capability snapshot per backend during verification.
      Acceptance: OpenGL, Vulkan (shared and dedicated queues), and D3D12
      expose queried values, unsupported graph operations are rejected against
      those values, and the FrameGraphTriangle regression remains unchanged.
      Implemented 2026-09-21: `GraphicsCapabilities` now includes the backend
      name, queried version and query-source snapshot. OpenGL derives feature
      booleans from the GL core version and enumerated extensions. Vulkan
      queries physical-device properties/features, extensions, format
      properties and compute queue families. D3D12 queries feature levels,
      options, root-signature and shader-model support plus color render-target
      and storage-image format support. The example logs the complete snapshot.
      Final VS2026 Debug (MSVC 14.50.35717) and Release (MSVC 14.51.36231)
      engine/example builds passed with warnings but zero errors. Debug and
      Release each ran OpenGL 4.6, Vulkan 1.4.341 shared queues, Vulkan 1.4.341
      dedicated queues, and D3D12 feature level 12.2 for 120 frames; every run
      logged its queried capability sources, passed frame-119 readback and
      exited with code 0. Unsupported subpass semantics remained rejected on
      OpenGL and D3D12. Vulkan validation layers were unavailable on this host;
      Vulkan therefore had no validation-layer coverage. Full migration of
      every remaining hardcoded limit accessor and capability coverage for
      less common resource variants remains open.

Recommended implementation order for the remaining graph work:
1. Fine-grained pipeline-stage scopes, subresource-range scheduling, and
   transfer-copy builders are implemented.
2. Pooling and aliasing only after lifetime and attachment semantics are
   verified across all three backends.

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
