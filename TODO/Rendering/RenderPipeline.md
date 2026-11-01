# Render pipeline framework

## Planned pipeline extensions (2026-10-31)

These are planning entries only; no implementation is included or authorized
by this TODO update. Preserve the completed lifecycle milestone below. Suggested
order: P1, P2, then P3; each milestone requires its own implementation evidence.

- [ ] P1 Pipeline assets and runtime switching.
  Scope: define a versioned engine-owned pipeline asset schema with stable pass
  type identifiers, ordered pass settings and resource references, plus a
  backend-neutral factory/registry for constructing supported pipeline types.
  Preflight schema, dependencies and backend capabilities before activation.
  Switch only between complete frames; retain the previous working pipeline
  if loading or configuration fails. Define camera/settings ownership across
  replacement and retain outgoing GPU resources through all relevant queue
  submissions. No per-frame device-wide wait or editor authoring in this scope.
  Prerequisite: the lifecycle/reset contract is implemented; review current
  multi-queue retirement paths before adding replacement ownership.
  Acceptance: asset round-trip and unknown-version/type/missing-resource errors;
  an example repeatedly switches between two visibly distinct pipelines and
  verifies deterministic GPU output, including failure rollback, resize,
  replacement and shutdown. Build engine/example in Debug and Release and run
  OpenGL, Vulkan and D3D12 separately with available validation, retirement and
  memory observations; document untested cases and unsupported capabilities.
  Update architecture/API pages and example guide. Status: planned, unimplemented
  and unverified; current pipelines are configured in C++.

- [ ] P2 Camera stacking.
  Scope: define an explicit base-camera/overlay-camera stack with stable camera
  references, deterministic order, viewport and target selection, color/depth
  clear/load/store rules and defined depth preservation. Specify inactive or
  missing cameras, incompatible targets/sample counts, duplicates and cycles;
  reject unsupported combinations explicitly. Record each camera through the
  graph with declared attachment dependencies; do not infer stacking from the
  existing Cameras vector. Define how a completed stack feeds presentation and
  post-processing, preserving existing single-camera behavior.
  Prerequisite: agree on camera identity and attachment ownership, and integrate
  with P1 selection/settings without storing native handles in assets.
  Acceptance: a focused example combines a base scene and an overlay with
  distinct output; numeric pixel checks cover ordering, viewport, clear versus
  preserved depth/color, disabled overlays and invalid stacks. Check target
  resize, pipeline replacement and shutdown; build/run Debug and Release on
  OpenGL, Vulkan and D3D12 separately. Synchronize architecture/API/example docs.
  Status: planned, unimplemented and unverified; passing camera data today does
  not automatically render a camera stack.

- [ ] P3 Ordered post-processing stack.
  Scope: define serializable effect settings and ordered enabled effects over
  graph-declared intermediate textures. Start with exposure and tone mapping;
  specify HDR input format/capability checks, linear color and output encoding,
  and apply the stack once after camera composition before presentation. Use
  separate input/output targets for effects; declare sampled/render-target or
  storage usages and same-state memory dependencies explicitly. Define bypass,
  parameter changes, effect reordering, resize and unsupported-format behavior.
  Pooling/aliasing, temporal history, bloom and editor authoring are outside the
  first milestone. Prerequisites: P1 settings/asset contract, P2 composition
  boundary and supported intermediate formats on each backend.
  Acceptance: an example exercises bypass, each effect, a two-effect chain,
  reordering and live settings changes with numeric reference pixels including
  values above 1.0 and documented tolerances. Verify resize/replacement/shutdown,
  stable intermediate-resource counts, Debug/Release builds and separate
  OpenGL/Vulkan/D3D12 runs with available validation; report unsupported paths.
  Update architecture/API/example docs. Status: planned, unimplemented and
  unverified; the current sampled presentation pass is not an effect stack.

Verification policy for P1-P3: use programmatic regression checks and logs;
desktop automation is excluded at the user's request. Record visible/manual
checks separately instead of inferring them from numeric GPU results.

## Lifecycle and rebuild follow-up (2026-10-31)

- [x] Correct configuration ownership and frame lifecycle before expanding the
  framework. Scope: configure every registered pass exactly once centrally;
  reject post-configuration pass mutations until explicit Reset; keep Execute
  and Present frame indices paired; reject invalid/reentrant phase calls in
  Debug and Release; recover failed configuration through Reset. Exercise
  externally registered passes, failure recovery, repeated graph rebuilds and
  changed offscreen dimensions in Example/RenderPipeline. No per-frame device
  wait, pooling, pipeline assets or editor authoring in this milestone.
  Acceptance: engine/example Debug and Release builds; OpenGL, Vulkan and
  D3D12 bounded runs pass lifecycle checks, compute readback before/after
  rebuild, dimension checks and normal shutdown; synchronized API/architecture
  and example guide. Manual window/minimize/visible and long-run checks remain
  separate. Initial inspection found pass Configure is never called by the
  framework, dirty reconfiguration appends duplicate graph declarations, and
  Present receives the next frame index.
  Implemented: the pipeline centrally configures registered passes in insertion
  order; explicit phase state rejects invalid/reentrant calls and pass mutation
  after configuration. Execute/Present now share a zero-based frame index that
  advances after Present. Reset clears passes, graph and frame index between
  frames; failed configuration requires Reset before retry. The example removes
  manual Configure calls, resolves producer versions during pass configuration,
  and rebuilds 512x512 -> 256x384 -> 640x360 targets at frames 30/60. External
  pass checks exercise the original unconfigured-pass failure, phase/mutation
  rejection, configuration-failure recovery and matching/reset frame indices.
  Verification: isolated CMake VS2022 engine/RenderPipeline Debug and Release
  builds passed with GE_WITH_MONO=OFF. OpenGL, Vulkan shared, D3D12 and Vulkan
  dedicated each passed 120 frames in both configurations, plus 6,000-frame
  Debug runs. Every run logged exactly three successful configurations, three
  compute readbacks (frames 29/59/119), both rebuilds and normal exit 0. Target
  dimensions are checked each frame. D3D12 used its debug layer; logs contain
  existing sampler warnings, no errors. Debug Vulkan did not log a validation
  fallback or validation errors; dedicated queues selected graphics=0,
  compute=2, transfer=5. Logs: build/pipeline-lifecycle/build-{Debug,Release}.log
  and runs/<configuration>-<backend>-<frames>/runtime.log; memory.csv records
  process samples. Changed Markdown links (47 targets) and git diff --check pass.
  Limits: repository Mono SDK preparation failed hashing the downloaded MSI;
  script-enabled builds were not tested and existing build settings were kept.
  Premake was not rerun (no discovery/build configuration changes). No visual,
  actual window resize/minimize or final-present pixel verification is claimed;
  desktop automation was stopped at user request. Six thousand frames took
  seconds, not a long-run memory test. D3D12 debug-layer samples after 3 seconds
  showed handles 4,059 -> 9,104 while private bytes were 165.8 -> 164.9 MiB;
  ownership remains unresolved under [D3D12 memory](../Integration/D3D12Memory.md).
  Pipeline assets/switching, camera stacking, post-processing and editor
  authoring remain future milestones; this closes only lifecycle/rebuild scope.

- [x] Reconcile TODO audit descriptions with current scene/script behavior.
  Acceptance: retain historical entries, identify removed scene physics paths,
  link native C++ script support, and explicitly identify legacy component
  helper tasks as superseded. Documentation review only; rendering API and
  backend ownership are unchanged by this reconciliation.

## First composable pipeline milestone

- [x] Rewrite the abandoned two-method `RenderPipeline` abstraction into a
      backend-neutral composable render pipeline framework, and add an example
      that actually uses it. Scope: replace the old
      `GEngine/Graphics/RenderPipeline.h` contract with a new renderer-owned
      pipeline/pass API under `GEngine/Renderer`; support ordered pass objects,
      a graph-facing builder, execution against `RenderGraph`, and an explicit
      presentation phase. Add a focused example that performs a real
      compute -> offscreen graphics -> sampled presentation chain on OpenGL,
      Vulkan, and D3D12. Do not claim full Unity URP/HDRP feature parity,
      asset-driven pipeline switching, camera stacking, post-processing stacks,
      or editor pipeline authoring in this milestone.
      Acceptance: the public API compiles in Debug/Release; the example builds
      and runs on all three backends for 120 frames; the offscreen result is
      numerically verified or visibly sampled into the final presentation path;
      pass ordering and present execution are exercised; architecture/API
      example documentation and this TODO are synchronized.
      Implemented: the old `GEngine/Graphics/RenderPipeline.h` contract is
      removed. `GEngine/Renderer/RenderPipeline.h` now provides
      `RenderPipelinePass`, `RenderPipelineBuilder`, `RenderPipelineContext`,
      and a composable `RenderPipeline` that owns a persistent `RenderGraph`.
      Derived pipelines create resources/targets and add ordered passes in
      `Configure`; `Execute` runs per-frame hooks and graph submission, while
      `Present` runs explicit presentation hooks. A focused
      `Example/RenderPipeline` implements `UpdateColorPass`,
      `GeometryPass`, and `PresentPass`, producing a real compute, offscreen
      graphics, and sampled presentation chain with a frame-119 numeric readback.
      Verification: CMake Debug/Release builds passed. OpenGL, Vulkan, and
      D3D12 each completed 120-frame Debug/Release runs; every run passed the
      storage-buffer readback and normal shutdown. D3D12 used the debug layer.
      Premake VS2022 regeneration and the Debug RenderPipeline build also
      passed. Premake runtime and Vulkan dedicated queues are not part of this
      first milestone. Remaining: manual visible pixel checks, pipeline
      assets/switching, camera stacking, post-processing stacks, editor
      authoring, resize/long-run behavior, and broader URP/HDRP-style features
      remain open.
