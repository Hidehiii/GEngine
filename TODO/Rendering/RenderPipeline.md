# Render pipeline framework

## Planned pipeline extensions (2026-10-31)

P1/P2/P3 implementation was authorized on 2026-11-01.
Preserve the completed lifecycle milestone below. Suggested
order: P1, P2, then P3; each milestone requires its own implementation evidence.

- [x] P1 Pipeline assets and runtime switching.
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
  Update architecture/API pages and example guide. Status: bounded v1 implemented
  and verified (2026-11-01). GAsset envelope with versioned composable-pipeline
  payload, named fixed-size color/depth targets, root-relative typed GAsset
  dependencies, scalar settings and ordered stable pass IDs. A runtime-owned
  pass registry validates before factory calls; a selector prepares a candidate
  fully before atomic activation, rejecting switches during frame/hooks and
  preserving the old pipeline/settings on load/configuration/preparation failure.
  Cameras remain caller-owned. Add RenderPipeline::Prepare and a read-only-graph
  pass preparation hook so presentation resources are created before activation.
  Registry callbacks and assets are snapshots; no global registry, editor,
  camera stack, effect stack, hot reload, pooling or device-wide frame waits.
  Example acceptance: two actual asset files with distinct RGBA output, repeated
  switching, immutable active settings, explicit resize by candidate replacement,
  unknown schema/pipeline/pass/dependency errors, configuration/preparation/cycle
  rollback and frame-boundary rejection. Numeric GPU output, normal shutdown,
  separate GL/Vulkan shared/dedicated/D3D12 Debug/Release and bounded stress,
  with lifetime/validation logs. Preserve original example mode and rerun it.
  Source review: D3D12 command generations retain resources until completion;
  Vulkan frame-fence command pools retain submitted pipelines, while native
  framebuffer retirement covers issued submissions;
  GL records immediately and native deletion retains pending uses. Existing
  backend retirement remains authoritative, not outgoing pipeline CPU lifetime.
  Required lifetime repair discovered during review: Vulkan BeginRenderPass
  lacks D3D12's explicit framebuffer owner retention. Recording against an
  outgoing target, replacing the CPU pipeline, then submitting that recorded
  command can otherwise release its framebuffer before submission. Retain the
  framebuffer in the command until pool reuse/completion. Add an example path
  that records an otherwise unused target before selection and submits after
  outgoing pipeline destruction; include native validation/discard/shutdown.
  Implementation progress: RenderPipelineAsset.h/.cpp adds strict YAML payload
  parsing/serialization, typed dependency and device target preflight, immutable
  target/factory snapshots, named pass creation and transactional selector.
  RenderPipeline Prepare compiles the graph and prepares pass resources once;
  failures preserve the old selection. Owning pipelines/selectors are noncopyable.
  Vulkan records a framebuffer owner. Example opt-in
  GENGINE_RENDER_PIPELINE_ASSET_CHECK loads Warm/Cool assets, checks round-trip
  and invalid candidates, switches every 15 frames, replaces warm output with
  137x83 after frame 60, verifies RGBA pixels and CPU pipeline release, and
  submits recorded outgoing-target work after replacement. Existing C++
  lifecycle/compute example remains the default. Build/runtime were pending at
  this initial implementation checkpoint.
  Initial Debug four-backend 120-frame asset runs passed 17 rejections, seven
  replacements and nine numeric checks each, with clean native diagnostics.
  Registry review follow-up: snapshot registrations before invoking validators,
  so a callback replacing the runtime registry cannot invalidate the active
  validation/factory iteration. Add a probe that clears its source registry
  during validation and still prepares the candidate from the captured factories.
  The snapshot probe and initial Debug 6000-frame runs passed all four variants
  (399 replacements/401 pixel probes each), with no native warnings/errors.
  Final lifetime refinement alternates submitting an outgoing recorded target
  with resetting/discarding that recording after CPU replacement; recheck final
  Debug/Release matrices, bounded stress and original lifecycle/compute mode.
  Required shader failure repair: Shader::Compile currently handles failed DXC
  compilation only with an assertion; GL/Vulkan compiler dispatch also overwrites
  the SPIR-V result with DXIL success. Failed candidate HLSL must throw before
  reflection/cache/native construction in both builds. Propagate both compile
  results, use owned DXC result/blob references on success/failure, and add an
  invalid-HLSL dependency rollback probe (18 total rejected candidates). Keep
  compiler warnings distinct from failure status. Broader legacy shader-container
  directive/parser hardening remains outside P1 and must not be claimed verified.
  Final verification: isolated GE_WITH_MONO=OFF CMake VS2022 GEngine and
  RenderPipeline Debug/Release builds passed. Final asset runs in
  build/pipeline-assets/final-{Debug,Release}/ each passed OpenGL, Vulkan shared,
  D3D12 layer-on and Vulkan dedicated at 120 frames. Each checked 18 failed
  candidates (including true HLSL compiler failure), registry self-replacement,
  seven switches, nine numeric center/corner RGBA checks, paired hooks/camera
  identity, 137x83 replacement, outgoing CPU release and native recorded work
  alternately submitted (four) or reset/discarded (three). Total: 144 rejection
  checks, 56 replacements, 72 pixel probes across eight backend/build runs.
  Debug Vulkan dedicated selected graphics=0, compute=2, transfer=5 without
  unavailable-layer fallback. No D3D12 native warnings/errors, Vulkan validation
  diagnostics or GL invalid operations were captured; the deliberate HLSL error
  is expected and followed by successful active-pipeline output, not suppressed.
  Final Debug 6000-frame runs passed all four variants, 399 replacements and
  401 pixel probes each (200 submitted/199 discarded outgoing recordings).
  Logs/CSV: build/pipeline-assets/final-stress-Debug/. Steady handle/private-byte
  samples: GL 557->557 / 156307456->156356608; Vulkan shared 799->798 /
  438751232->438689792; D3D12 912->1193 / 170979328->171393024;
  Vulkan dedicated 834->834 / 450682880->450953216. These are bounded workload
  observations; the separate compute-heavy D3D12 validation-growth diagnosis
  remains open. CPU pipeline weak references expire after replacement; no
  automatic retired pipeline list or per-frame device wait was added.
  Original C++ lifecycle/compute mode also passed all four backend variants in
  Debug/Release at 120 frames, with three configurations/readbacks, failure
  recovery and normal shutdown (build/pipeline-lifecycle/runs/). Application DLL
  hashes match engine outputs. CMake CONFIGURE_DEPENDS/Premake source globs cover
  new files; no generated-project-only edits. Premake, Mono-enabled integration,
  archive runtime reads/export dependency discovery, manual/final-window pixels,
  injected OOM/device loss and indefinite memory remain unverified. Legacy
  shader-container parser hardening is explicitly tracked in ShaderGraph.md.
  P2 camera stacking and P3 effects are not implemented by this milestone.
  Final cold-cache baseline recheck used fresh 121-frame run directories in
  Debug/Release on all four variants, covering valid compute compilation as
  well as configuration/rebuild/readback. No native diagnostics appeared.
  Python syntax and 69 changed-document relative links pass; git diff --check
  passes. Unstaged review refreshed; original Config.ini selections and tmp/
  are untouched. No commit/push was requested for this new milestone.

- [x] P2 Camera stacking.
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
  Status: bounded implementation and programmatic verification complete
  (2026-11-01). Bounded P2: version-2
  pipeline payloads add named camera stack trees with stable application camera
  IDs, one explicit base, ordered overlay edges, enabled flags, normalized
  top-left viewports, inherited target/sample constraints and whole-attachment
  clear/load rules (STORE fixed). Keep v1 assets unchanged. Validate duplicates,
  cycles/shared overlays, disconnected nodes, role/target/sample/depth/viewport
  mismatches before candidate activation. Missing active bindings fail before
  submission; inactive parents suppress descendants and inactive bases clear
  output deterministically. Bindings borrow caller cameras through Execute only.
  Add reusable CameraStackPass plus per-camera graph access/dependency declarations
  and terminal composed output for presentation/future effects. No scene camera
  discovery, camera asset authoring, multiple-target composition or P3 effects.
  Required foundation: compatible immutable render-pass override at command begin
  (retain pass+framebuffer through submission), and pixel viewport/scissor API on
  all three backends. Vulkan draws must honor the command viewport; GL clears,
  resolves and later presentation must not inherit overlay scissor clipping.
  Example acceptance: actual asset-defined stack, base/depth-overlay/UI colors,
  depth LOAD versus CLEAR, whole-target color clear, top-right viewport/scissor,
  ordering, disabled/missing bindings, invalid stacks and failed P1 switching,
  1x/4x, resize/replacement, shutdown. Debug/Release GL/Vulkan shared/dedicated/
  D3D12 runs with numeric pixels, native validation and bounded stress; rerun P1
  and original single-camera pipeline. No computer-use automation or device idle.
  Initial Debug GL stack runs passed all pixels/preflight/binding checks. Vulkan
  pixels passed but native validation rejected the first write after each target
  creation: borrowed color/depth wrappers claim attachment-optimal while new
  images are actually UNDEFINED. Manual framebuffer passes expose this through
  graph barriers that auto-target passes previously skipped. Track new offscreen
  attachments as UNDEFINED and transition on actual use; recheck both stack and
  existing framebuffer/MSAA paths. Also return VK_FORMAT_UNDEFINED from the legacy
  RenderImage2DFormat converter for unrepresented depth formats rather than falling
  off its return path during attachment transitions. This is failed first-run
  evidence, not validation success. No layout diagnostic will be filtered out.
  Debug/Release four-backend 120-frame stack runs passed after the layout repair;
  P1 and original Debug mode also passed. Final example refinement toggles UI
  activity on frames 1/2 without replacing the instance, checking both callback
  order and pixels; stack acceptance now requires 11 probes at 120 frames / 403
  at 6000, including two same-instance live-binding markers. MSAA regressions
  recheck the attachment initialization repair independently on Vulkan queues.
  Required presentation integration: raw quadrant checks reveal that the existing
  presentation quad's bottom-up UVs invert composed attachments on top-origin
  backends. Expose semantic AttachmentUvStartsAtTop in device capabilities (GL
  false, Vulkan/D3D12 true), not API-identity branches in renderer code. Use it
  for the example's offscreen sampled-output consumer and final presentation;
  read back both raw and sampled output so orientation and viewport reset are
  verified before claiming a stack feeds presentation. This is a sampling copy,
  not a P3 effect. Include complete rebuilds for the capability snapshot ABI.
  Final-source Debug checks passed on GL, Vulkan shared, D3D12 and Vulkan
  dedicated: 120 frames, 16 preflight rejections, three binding rejections,
  two live activity changes on the same pipeline, seven replacements and 11
  paired raw/sampled quadrant probes each. Logs: build/camera-stacks/final-Debug.
  Final Debug stress also passed 6000 frames per variant, 399 replacements and
  403 paired probes each (build/camera-stacks/final-stress-Debug), with clean
  native diagnostics and normal shutdown. Steady handles/private bytes:
  GL 558->558 / 154767360->155578368; Vulkan shared 793->785 /
  413147136->413401088; D3D12 796->901 / 173658112->175214592;
  Vulkan dedicated 834->827 / 422486016->422993920. These are bounded samples,
  not proof of indefinite stability or resolution of the separate D3D12 issue.
  Final Debug P1 regression passed all four variants (18 rejections/seven
  switches/nine probes); original C++ mode passed fresh 123-frame runs with
  three configurations/readbacks each and no captured native diagnostics.
  Final-source Release stack runs also passed all four variants at 120 frames
  with the same 16/3/2/7/11 acceptance counts, including numeric raw and sampled
  output (build/camera-stacks/final-Release). Combined Debug/Release stack runs
  checked 128 invalid candidates, 24 invalid bindings, 16 same-instance activity
  markers, 56 replacements and 88 paired-output probes. GEngine/RenderPipeline
  and FrameGraphTriangle rebuilt successfully in both configurations using
  isolated VS2022 CMake GE_WITH_MONO=OFF; all owned builds/tests finished.
  Release P1 regression passed all four variants (final-p1-Release); original
  C++ mode passed fresh 123-frame runs in both builds with clean native output.
  Rebuilt FrameGraphTriangle --msaa passed all four variants in Debug/Release
  (build/camera-stacks/final-msaa-{Debug,Release}), covering format preflight,
  MRT/depth resolve, preserved LOAD, target replacement and three GPU readbacks.
  Vulkan shared and dedicated queues independently passed after the initial
  layout repair; no validation-unavailable fallback or native warnings/errors.
  Shader parser hardening, Mono/Premake runtime integration, archive/export
  dependency discovery, manual final-window pixels/window interaction, injected
  OOM/device loss and indefinite memory stability remain unverified/out of scope.
  CMake/Premake source globs cover the added engine/example files; no generated
  project-only edits. P3 effects remain planned. No computer-use automation,
  commit or push; original Config.ini selections and preexisting tmp/ preserved.
  Handoff checks: 72 changed-document relative links and runner syntax passed;
  engine DLL hashes match both examples in Debug/Release. Diff reviewed and
  git diff --check passed; unstaged review refreshed, staging remains empty.

- [x] P3 Ordered post-processing stack.
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
  Update architecture/API/example docs. Status: bounded implementation and
  programmatic verification complete (2026-11-01). Bounded P3: payload v3 adds named ordered effect
  stacks; v1/v2 remain compatible. Add RGBA16F HDR attachments and native format
  queries/mappings on all three backends, preserving existing enum values.
  Input is one resolved linear RGBA16F attachment; output is a distinct same-size
  single-sample RGBA8 target without depth. Intermediate targets are RGBA16F,
  graph-owned, fixed per configured stage and never pooled/aliased. Exposure
  multiplies nonnegative RGB by exp2(EV), EV finite in [-16,16]; Reinhard tone
  mapping is x/(1+x). Preserve alpha. Explicit Linear or standard piecewise
  sRGB encoding is applied once at the last stage into a UNORM target, with no
  implicit hardware sRGB conversion. Empty/disabled effects copy with the chosen
  output encoding. Shader resource uses a validated stable material contract.
  Live provider changes enable/EV/order/encoding before graph recording without
  changing stage count or allocating targets; resize/count changes use P1
  candidate replacement. Reject invalid topology, alias, formats, encoding,
  duplicate/unknown effects and nonfinite parameters before activation/recording.
  Example composes two camera views into HDR, runs effects once after composition,
  checks CPU reference quadrants for bypass, each effect, chain/reorder/live
  changes, sRGB and resize, includes >1 inputs and graph target-count checks.
  Require Debug/Release engine/example builds and four backend variants, bounded
  Debug stress/native diagnostics and P1/P2/default/MSAA regressions. No compute,
  desktop automation, bloom/history, pooling, automatic export or editor work.
  Source progress: PostProcess.h/.cpp implements validation, registered graphics
  pass factory, separate fixed graph stages/materials, live settings snapshots,
  explicit intermediate ownership and shader contract preflight. v3 serialization
  coexists with v1/v2, and native RGBA16F mappings/queries preserve existing enum
  values. Exposure clamps to 65504 to keep half-float intermediates finite;
  alpha is preserved. Nearest/clamp sampling avoids assuming linear filtering.
  The example deploys the v1 shader resource and checks exact graph-owned target
  counts/identity each frame (diagnostic readback arrays are separate). It uses
  12 command slots in isolated copied config, requiring at least nine for the
  two-camera/two-effect/terminal-reader/completion plus readback graph workload;
  original backend selections/config remain untouched. Cases 1/2 exercise actual
  single-effect topology; case 5 changes settings/order without replacing two
  slots; case 6 is empty, case 7 restores the chain. Expected 120-frame counts:
  18 rejected candidates, four rejected live settings before any camera callback,
  two same-instance markers, six replacements and 11 quadrant probes. 6000 frames
  require 349 replacements/403 probes. Initial Debug runs passed all four variants
  with clean native diagnostics (build/post-process/attempt-Debug); Release and
  bounded stress/regression evidence are pending at this checkpoint.
  Final evidence: isolated VS2022 CMake GE_WITH_MONO=OFF GEngine/RenderPipeline
  and FrameGraphTriangle Debug/Release builds passed; all owned processes ended.
  Initial native source matched the successful Debug matrix above; the final
  pass-reuse refinement and its verification are recorded below.
  Release P3 matrix passed all four variants in build/post-process/final-Release:
  120 frames, 18 invalid candidates, four invalid live settings, two live markers,
  six replacements and 11 reference probes each. Combined Debug/Release P3:
  144 rejected candidates, 32 rejected live snapshots, 16 same-instance markers,
  48 replacements and 88 four-quadrant probes, including alpha and sRGB toe.
  Every frame checked expected graph-owned target count/identity. No native
  warnings/errors or Vulkan validation-unavailable fallback were captured.
  Debug stress passed 6000 frames on GL, Vulkan shared, D3D12 layer-on and Vulkan
  dedicated, 349 replacements/403 probes each; logs/CSV: build/post-process/
  stress-Debug. Steady handles/private bytes: GL 555->555 / 156237824->156422144;
  Vulkan shared 795->797 / 440401920->440885248; D3D12 843->1157 /
  176287744->177729536; Vulkan dedicated 838->838 / 449945600->450076672.
  These are bounded observations, not proof of indefinite stability or a fix
  for the separately tracked compute-heavy D3D12 validation handle growth.
  P1/P2 regressions passed all four variants in both builds at 120 frames
  (build/post-process/p1-{Debug,Release}, p2-{Debug,Release}). Original C++ mode
  passed fresh 124-frame runs, three configurations/readbacks per variant/build,
  with clean native diagnostics. Rebuilt FrameGraphTriangle --msaa passed all
  four variants in both configurations (build/post-process/msaa-{Debug,Release});
  format 10 independently reports queried RGBA16F attachment/sample/resolve
  capability and validates its 1x framebuffer metadata. Existing MRT/depth,
  preserved LOAD, resize/replacement and readback checks also passed.
  Limitations: manual final-window/display encoding, Mono/Premake runtime,
  archive/export dependency discovery, injected OOM/device loss, three/four-slot
  effect chains and numeric RGBA16F MSAA resolve remain untested. P3 uses 1x HDR
  and verifies zero/one/two slots; HDR swapchains, premultiplied-alpha correction,
  bloom/history/pooling/editor UI are outside this bounded milestone. Required
  formats were available here; unsupported hardware rejection is source-inspected,
  not simulated by changing device capabilities. No commit/push or desktop
  automation; preexisting P1/P2 edits, backend selections and tmp/ preserved.
  Final review follow-up: reusing a PostProcessStackPass for an equivalent asset
  graph currently appends old source/material/intermediate vectors on Configure.
  A second Prepare can retain old attachments and index beyond the configured
  effects. Clear prepared/configuration state before declaring the new graph;
  add a two-instance same-pass preparation regression and recheck final builds
  and P3 native/stress results. Changed topology still requires a new context/
  candidate; this regression reuses an equivalent immutable asset layout only.
  Follow-up complete: Configure clears old source/intermediate/material/pipeline
  references. The example prepares two successive candidates using the same
  pass and equivalent layout, requires exactly one intermediate each time, then
  releases both without GPU recording. Final Debug/Release builds and four-backend
  120-frame runs passed this additional `Post-process pass reuse passed` marker,
  plus all 18/4/2/6/11 checks; authoritative logs are now build/post-process/
  final-{Debug,Release}. Final Debug stress also passed 6000 frames per variant,
  including reuse preflight and 349 replacements/403 probes each:
  build/post-process/final-stress-Debug. Steady handles/private bytes:
  GL 558->558 / 154796032->154943488; Vulkan shared 799->800 /
  441466880->441528320; D3D12 881->1400 / 175534080->176553984;
  Vulkan dedicated 835->836 / 448348160->448401408. Exact graph-owned target
  counts/identity and native diagnostics passed; D3D12 process-handle growth
  remains explicitly unresolved. Earlier P1/P2/default/MSAA regressions preceded
  this post-only reconfiguration fix; their exercised code paths are unchanged.
  Both examples were rebuilt against final engine DLLs, with matching hashes.
  Handoff checks passed: 78 relative documentation links, runner syntax and
  git diff --check. Diff reviewed; unstaged review refresh requested (queued by
  the app). Staging is empty.

Verification policy for P1-P3: use programmatic regression checks and logs;
desktop automation is excluded at the user's request. Record visible/manual
checks separately instead of inferring them from numeric GPU results.

## P1-P3 commit and push handoff (2026-11-01)

User authorized committing and pushing the completed P1-P3 work. Scope: stage
only the implementation, focused examples/regression runner and synchronized
documentation above; exclude preexisting tmp/ and preserve local backend config.
Acceptance: inspect the staged diff, pass whitespace checks, commit the reviewed
snapshot, push master to origin without force, and verify local/remote commit
identity and remaining worktree state. Architecture/API behavior is unchanged
by this handoff; implementation and verification evidence remain recorded above.
Staging checkpoint: the reviewed 68-file manifest matches the index, staged
whitespace checks pass, and fetched origin/master matches the pre-commit HEAD.
Only preexisting tmp/ is excluded from the worktree snapshot; backend config is
unchanged. The resulting commit hash and remote verification are reported in
the handoff rather than embedded in their own commit.

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
