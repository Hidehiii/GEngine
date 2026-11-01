## Phase 4 — device-owned creation and modules

## Per-format capabilities and MSAA (2026-11-01)

- [x] Complete device-owned per-attachment-format queries and supported MSAA
  color resolve. Scope: report native attachment/sample support, sampled output,
  dimensions and resolve capabilities for every existing FrameBufferTextureFormat;
  preflight the intersection for all color/depth attachments before allocation,
  including direct device factories and graph compatibility input. Repair GL
  multisample texture creation/MRT resolve, D3D12 multisample views and illegal
  fixed-function depth resolve, and Vulkan attachment formats/sample allocation.
  Preserve borrowed/owning resource boundaries and submission retirement through
  construction, replacement, resize and shutdown. Unsupported formats, counts or
  depth-resolve requests must fail explicitly in Debug and Release. No implicit
  sample-count downgrade, per-frame device wait, new asset format or pooling.
  Acceptance: log every format query on GL/Vulkan/D3D12; reject unsupported
  combinations before new graph allocations; render and resolve real geometry
  using supported 1x/2x/4x/8x counts, MRT and depth attachments, verify pixels and
  fractional edge coverage, rebuild at different dimensions and sample counts,
  then shut down normally. Engine/examples Debug/Release build and runtime
  matrices, available native validation and bounded resource/memory observations
  must be recorded separately. Update API/architecture, example guide and tests.
  Initial source evidence: GL allocates a multisample texture using a single-
  sample attachment path; D3D12 creates TEXTURE2D views for MSAA and calls color
  ResolveSubresource on depth; Vulkan reuses the preceding depth format for
  MSAA color images. Implementation and verification pending; no desktop UI.
  Regression-discovered blocker: the D3D12 texture-array descriptor omitted an
  explicit mip count, allocating a full chain while metadata/index calculations
  assumed one mip. The new 31x29 MRT layer-1 copy targets native mip 1 (15x14)
  and emits a CopyTextureRegion bounds error. Set native MipLevels=1 to match
  the public one-mip array contract; rerun both existing array-layer checks and
  the MSAA readbacks in Debug/Release. This focused fix is part of MSAA verification.
  Per-format metadata follow-up: framebuffer wrappers must report NONE when
  the existing RenderImage2DFormat schema cannot represent their native format.
  D3D12 currently labels every wrapped attachment RGBA8, while Vulkan's conversion
  lacks a default return. Normalize these paths and instantiate each supported
  single-sample color format so copy preflight cannot accept a false RGBA8 match.
  D3D12 MSAA runtime also rejected ALLOW_ONLY_RT_DS_TEXTURES on committed
  resources; those allocation flags are for explicit heaps. Use HEAP_FLAG_NONE
  for committed MSAA color/depth allocations and recheck native validation.
  Diagnostic sequencing correction: three ExecuteGpu calls occur within one
  presentation frame, before its external completion command is submitted.
  End each MSAA diagnostic graph with a graphics ShaderRead consumer after the
  transfer copy, closing graphics -> transfer -> graphics and restoring sampled
  states before graph reuse. Native errors from the earlier copy-only terminal
  graph are failed test-design evidence, not successful synchronization coverage.
  Implemented: RenderDevice::QueryRenderTargetFormat and ValidateRenderTarget
  expose per-format roles, unfiltered sampling, sample masks, size bounds,
  color/depth resolve and preserved-MSAA-load capability. GL queries single/
  multisample internalformat and framebuffer limits; Vulkan intersects image
  usages/format properties/framebuffer limits and depth resolve modes; D3D12
  queries format flags and per-count quality levels. Graph and direct factory/
  resize paths reject unsupported combinations before allocation. MSAA images,
  views, MRT resolve, depth clear aspects, D32 resolve mode, committed allocation
  flags and array mip indexing are corrected. Aliasing texture references retain
  framebuffer ownership through graph Reset and later submissions. Partial
  construction releases tracked native views/images/descriptors.
  Verification: CMake VS2022 GE_WITH_MONO=OFF GEngine/FrameGraphTriangle
  Debug/Release builds passed. GL, Vulkan shared, D3D12 layer-on and Vulkan
  dedicated each passed 120 frames in both configurations with three matrices
  (frames 0/30/60), 24 cases each, 18 multisample cases each. Counts 1/2/4/8,
  MRT=2, no depth/D24S8/D32, 31x29/47x33, repeated execution, direct resize,
  depth occlusion, fractional color coverage, retained owners, unsupported
  count rejection and ordinary array readbacks passed. Every supported color
  format was created at 1x and metadata checked. D3D12 depth-output rejection
  and Vulkan MSAA LOAD rejection passed; GL/D3D12 preserved LOAD rendered
  correctly. Numeric resolved-depth values are not claimed. Final logs:
  build/msaa/verified-{Debug,Release}/. All exited 0 without D3D12 warnings/errors
  or Vulkan validation diagnostics. Debug Vulkan had no unavailable-layer
  fallback; dedicated families were graphics=0, compute=2, transfer=5. GL had
  existing readback performance logs. RGBA8 masks were GL=63 (1..32), Vulkan/
  D3D12=15 (1..8); counts above 8 were queried, not rendered by this matrix.
  Debug 6000-frame matrices passed all four variants with normal shutdown.
  GL was rechecked after including multisample texture dimensions in the query
  (build/msaa/final-limit-stress-Debug, Release 120 frames in final-limit-Release).
  Other stress logs/CSV: build/msaa/stress-Debug/. GL/Vulkan steady handles were
  near-flat; D3D12 layer-on handles grew 5491->32345 and private bytes were
  175616000->176979968, consistent with the separate unresolved diagnosis.
  No queue error was masked by a per-frame device idle; diagnostic readbacks
  intentionally synchronize. Existing Premake src/**.cpp discovery and CMake
  CONFIGURE_DEPENDS cover MsaaChecks.cpp. Premake/script-enabled builds, actual
  window/final-present pixels, injected OOM and indefinite memory remain unverified.
  Existing RenderPipeline was rebuilt against the final interface in both
  configurations; its GL/Vulkan shared/D3D12/Vulkan dedicated 120-frame
  lifecycle/rebuild/readback matrix passed separately. Runtime DLL hashes were
  checked against the current engine output. Final link/content review checked
  87 relative targets, Python syntax and native diagnostic logs; git diff --check
  passes. Changes remain uncommitted and original backend settings are preserved.

- [x] Complete Vulkan preserved MSAA LOAD (authorized 2026-11-01): retain multisample contents through
  store/layout transitions and expose MultisampleLoad=true only when supported.
  Acceptance: later load-only passes preserve color/depth numerically across
  frames with shared/dedicated validation, resize and shutdown. Current MSAA
  LOAD is explicitly rejected in the preceding milestone. New scope: persistent
  device-local backing, STORE/layout dependencies, load-only color and depth
  regression without redrawing the original geometry; both queue configurations.
- [x] Complete D3D12 depth resolve (authorized 2026-11-01): define portable depth/stencil resolve modes
  and select supported native/shader paths with explicit usage queries.
  Acceptance: sample known depth into color and verify values, reversed depth,
  supported formats/counts, resize and queue lifetime. Until implemented,
  DepthResolve=false and MSAA GetDepthStencil throws; depth testing and resolved
  color remain supported in the preceding milestone. New scope: native graphics
  ResolveSubresourceRegion, queried min/max modes, typeless sampled depth views,
  explicit default semantics per backend and rejection of unsupported modes.
  No compute pass or desktop automation. Add numeric depth sampling into RGBA8,
  conventional/reversed depth, load-only passes and bounded Debug/Release runs.
  Initial implementation record (verification was pending): DepthResolveMode and per-format mode masks,
  graph legacy/portable forwarding and render-pass cache equality; D3D12
  OPTIONS2 Tier1 gate, typeless depth resources and typed sampling views,
  graphics-list min/max depth and min stencil resolves. Vulkan persistent
  backing, STORE, LOAD initial layouts and attachment read/write dependencies;
  queried sample-zero/min/max combinations account for independent stencil
  resolve. Existing framebuffer ownership/retirement remains in use.
  Example now samples depth into RGBA8 and reads pixels, runs both depth
  conventions and only far geometry after initial stored near geometry.
  At that stage the first full build was running; an early old-binary diagnostic is not new-source
  evidence. Script execution policy is handled only in the child process.
  Reverse-depth source review found D3D12 offscreen/present viewports invert
  MinDepth/MaxDepth, although depth convention already belongs in projection,
  clear and compare. Keep the native viewport range [0,1] so known 0.8 reversed
  geometry retains its depth; exercise the reversed numeric/occlusion regression.
  First native Debug D3D12 run completed numeric depth but failed strict native
  diagnostics because reverse-depth optimized clear values were hardcoded to 1;
  creation now matches the active clear convention. Vulkan shared passed the
  initial within-frame numeric test. A dedicated test accidentally paired the
  old binary with an updated shader; its owned process was stopped and that log
  is failed fixture evidence, not synchronization evidence. Final runner now
  requires four actual-frame LOAD probes (1/59/61/119), with target recreation
  at 60, retained color+depth checks and explicit min/max mixed-coverage checks.
  Intermediate Debug D3D12 and Vulkan dedicated checks passed these new probes
  and native diagnostics. OpenGL's shader converter does not handle the probe's
  samplerless Load path; use existing separate textures/nearest samplers and
  pixel-center SampleLevel in the diagnostic instead. This changes the probe,
  not the engine's advertised shader capabilities. Final full matrices pending.
  The portable sampled probe passes GL/Vulkan but exposes D3D12 sampler warnings:
  material fallback and explicit standard samplers set ComparisonFunc=ALWAYS
  despite a non-comparison filter. Use NEVER in both descriptors (ignored by
  standard sampling) and require warning-free explicit/fallback sampling in the
  final Debug/Release matrices. No comparison-sampler API is added.
  Final verification: GE_WITH_MONO=OFF CMake VS2022 GEngine/FrameGraphTriangle
  Debug/Release builds passed. GL, Vulkan shared, D3D12 layer-on and Vulkan
  dedicated each passed 120 frames in both configurations, three matrices each:
  GL 48 cases/matrix, Vulkan 72 each, D3D12 64, totaling 1536 MRT color cases,
  1184 numeric depth probes (including 32 actual-frame probes) and 432 min/max
  mixed-coverage checks. D24S8/D32, 1/2/4/8x, conventional/reversed depth,
  explicit queried modes at 4x, unsupported mode rejection before allocation,
  load-only/far-only draws, resize and owner retention all passed. Persistent
  fixtures preserve stored color/depth across frames 1/59/61/119, recreating
  targets at 60. No D3D12 warnings/errors, Vulkan validation diagnostics or GL
  invalid operations were captured. Final logs: build/msaa-depth/verified-
  {Debug,Release}/<backend>/runtime.log; DLL hashes match current engine outputs.
  Debug 6000-frame runs also passed all four variants and normal shutdown:
  build/msaa-depth/stress-Debug/. Steady sampled handles/private bytes were
  GL 557->557 / 174632960->166019072, Vulkan shared 791->793 /
  414507008->414339072, D3D12 1577->41170 / 177152000->179167232,
  Vulkan dedicated 836->837 / 425328640->424636416. Dedicated families were
  graphics=0, compute=2, transfer=5; Debug had no unavailable-layer fallback.
  D3D12 validation-layer handle growth remains the separate open Integration
  diagnosis, not a claim of stable indefinite memory. New waits are diagnostic
  readbacks only; no per-frame device-wide wait or compute resolve was added.
  Remaining verification limits: native depth resolve requires D3D12 OPTIONS2
  Tier1+ (Tier0 is explicitly unsupported, no shader fallback); stencil modes
  are defined but stencil numeric sampling is not exposed/tested; >8x rendered
  samples, injected OOM, Premake/Mono-enabled builds, manual final-window pixels
  and broad hardware coverage remain unverified. These do not hold the bounded
  tested depth/LOAD milestone open. Earlier failed logs are not acceptance runs.
  Existing RenderPipeline was rebuilt against the final engine in Debug/Release;
  application DLL hashes match engine outputs. All eight 120-frame backend runs
  passed configuration guards, failure recovery, three target generations and
  three readbacks, with normal shutdown. Logs: build/pipeline-lifecycle/runs/.
  Final content/link/syntax review and git diff --check pass; original backend
  selections and tmp/ are preserved, and changes remain uncommitted.

## Foundation closeout milestone (2026-10-31)

- [x] Reconcile historical foundation summaries and close concrete attachment/
  capability validation gaps. Scope: preserve historical evidence but point old
  stage/range/transfer TODO summaries to current implemented contracts; validate
  portable and legacy graph attachment descriptions consistently; preflight
  target limits and unsupported subpasses before any transient allocation.
  Expose MaxColorAttachments from backend limits (queried GL/Vulkan, native
  required D3D12 limit), and correct D3D12 uniform-buffer capacity reporting
  from 16-byte element counts to bytes. No removal of compatibility adapters,
  pooling, aliasing, general exclusive ownership or backend module redesign.
  Acceptance: focused FrameGraphTriangle regressions reject malformed formats,
  sample counts, operations, subpass indices and device-limit violations in
  Debug/Release, including legacy input; unsupported graph validation leaves
  transient resources unallocated. Build engine/example in Debug/Release, run
  OpenGL/Vulkan shared/dedicated/D3D12 separately with three GPU readbacks and
  normal shutdown; record capability limits and validation output. Update API,
  architecture, example and TODO evidence. Add a reusable native-example runner
  under Tests for isolated backend runs and matched lifetime controls; document
  its arguments, output and limitations in Tests/README.md.
  Implemented: both graph target overloads use the same structural validation;
  invalid colors/depth/sample counts/operation enums/subpass indices are rejected
  before declarations are appended. Compile preflights every target's texture
  dimensions, color count, framebuffer/render-pass capabilities and subpass
  support before allocating transient resources or targets. OpenGL queries both
  color-attachment/draw-buffer limits, Vulkan uses maxColorAttachments, and
  D3D12 reports its native required count with a diagnostic source. D3D12's
  uniform limit now reports 4096 * 16 = 65536 bytes. Historical summaries in
  ArchitecturePhase1/2, FrameGraphTransitions and Milestones link current evidence.
  Verification: isolated GE_WITH_MONO=OFF CMake VS2022 GEngine/FrameGraphTriangle
  Debug and Release builds passed. OpenGL, Vulkan shared, D3D12 layer-on and
  Vulkan dedicated each completed 120 frames in both configurations; all passed
  26 malformed declaration cases, allocation-free target preflight, three GPU
  readbacks and normal exit 0. Each reports MaxColorAttachments=8 and uniform
  bytes=65536; max texture dimensions are GL/Vulkan 32768, D3D12 16384. Debug
  Vulkan dedicated selected graphics=0, compute=2, transfer=5; no validation
  fallback or native warnings/errors appeared. Expected graph-cycle diagnostics
  are separate negative tests. Tests/RenderingFoundation.py provides isolated
  native matrix and lifetime modes; Python syntax/link checks pass.
  RenderPipeline was also rebuilt in Debug/Release against the final graph code;
  its four backend variants passed 120-frame lifecycle/rebuild/readback runs
  separately in both configurations. The existing sampler warnings remain.
  Initial regression attempt incorrectly used the compiled-only framebuffer
  getter after expected rejection; Debug timed out and was cleaned up, Release
  exited 0xc0000409. The check now inspects attachment resource references and
  exact rejection reasons; final Debug/Release matrices passed. Build logs are
  build/pipeline-lifecycle/build-FrameGraphTriangle-{Debug,Release}.log; final
  runs/logs/CSV are build/foundation/{Debug,Release}/<backend>/.
  Remaining: native per-format/sample queries, framebuffer-specific dimension
  limits beyond texture limits, full legacy migration and broader resource
  capability coverage stay open. Structural power-of-two sample validation is
  not a hardware MSAA-support promise. Script-enabled/Premake builds, visual
  pixels and actual window operations were not revalidated; no build discovery
  changes or desktop automation were made.
  Superseded scope note (2026-11-01): the per-format/MSAA milestone above now
  implements attachment-format queries and framebuffer-specific dimension
  bounds. Broader resource variants and full legacy migration remain open.
- [x] Extend current-source lifetime diagnosis with matched D3D12 frame-count
  controls, layer on/off and replacement on/off, using isolated configurations
  and process memory/handle samples. Review existing trace attribution before
  assigning ownership. Acceptance: bounded current-source runs complete with
  readbacks and normal shutdown, record pending retirement/live pipeline counts
  where available, and distinguish observations from attribution. Do not repeat
  elevated system tracing or introduce device waits. Unresolved allocation
  ownership stays open in [D3D12Memory](../Integration/D3D12Memory.md); no desktop
  automation.
  Verification: current Release source, real debug layer off/on and replacement
  off/on, 30,000 frames per case; all four passed foundation checks, all three
  readbacks and normal shutdown/exit 0. Layer-off runs took 21.5/19.7 s and
  layer-on runs 56.1/54.1 s; this matches frame count, not elapsed time. Steady
  samples exclude startup (<3 s) and the last second of teardown. Layer off:
  handles 706->706 (no replacement), 703->707 (replacement); private bytes
  146.01->146.30 / 143.94->145.23 MiB. Layer on: handles 14819->271487 /
  15217->281367; private bytes 164.45->180.43 / 165.26->181.63 MiB.
  Sampled pending submissions were at most 15, deferred releases and stored/
  discarded debug messages stayed 0. Replacement traces reported created=1000,
  live=3. Layer-on context reports contain bounded context-owned queues/fences/
  allocators; no-replacement/replacement resource and PSO counts differ at that
  point (11/13 resources, 5/8 PSOs). This is not a post-device teardown report
  or proof of leak freedom. Growth persists without replacement, but native
  handle type/allocation owner remains unresolved. Evidence is under
  build/foundation/lifetime-Release/; the parent D3D12Memory TODO stays open.

## Existing Phase 4 milestones

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
