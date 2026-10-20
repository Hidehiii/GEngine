## Component and helper rendering gaps found during documentation audit

- [ ] Implement and verify D3D12 swap-chain resize before claiming interactive
  resize support. Source inspection on 2026-10-11 found that
  `EngineRuntime::OnWindowResize` updates only the graphics viewport, while
  the D3D12 presentation/context path has no `ResizeBuffers` call and keeps
  its existing back-buffer targets. Acceptance: a nonzero size change
  recreates dependent targets after GPU-safe retirement, rendering continues
  at the new size, zero-size/minimize skips unsafe recreation, and restore plus
  normal shutdown pass with the D3D12 debug layer. This is a discovered
  limitation, not a verified runtime failure; no window interaction was run.
  Bounded implementation milestone (2026-10-18): route nonzero runtime resize
  through the presenter, wait for D3D12 graphics/compute/transfer submissions,
  release presenter command lists and old back-buffer views/resources, call
  `ResizeBuffers`, then recreate targets and presenter command lists. Skip
  zero-size events and leave other backends' resize behavior unchanged.
  Acceptance for this milestone: Debug and Release engine/example builds,
  noninteractive D3D12 startup/readback/shutdown with the debug layer, and
  source-level lifetime review. Interactive size/minimize/restore remains open
  until manually checked without desktop automation.
  Implemented: `RenderSystem` forwards nonzero resize to the presenter after
  dropping the previous presentation graph. D3D12 waits for all queues,
  releases presenter command lists and old context targets, resizes buffers,
  then recreates targets and command lists. Zero sizes are ignored; other
  backends keep their existing resize handling. Visible resize/minimize/restore
  is not yet verified.
  The first Debug build exposed an incorrect `RenderDevice::WaitForIdle` call;
  it was replaced with explicit waits on the context's three queue fences
  followed by deferred-release collection. The subsequent Debug build passed.
  Noninteractive regression extension: add an opt-in FrameGraphTriangle
  synthetic nonzero resize sequence at frame boundaries (without changing the
  native window). Acceptance: D3D12 Debug Layer and Release runs rebuild
  targets twice, reach frame 120 with both GPU readbacks and normal shutdown;
  default example behavior remains unchanged. This tests `ResizeBuffers`, not
  actual OS resize/minimize/restore.
  Implemented: `GENGINE_EXAMPLE_SYNTHETIC_RESIZE=1` emits 960x540 and
  1280x720 events after frames 30 and 60 on D3D12. The context logs only
  after successful `ResizeBuffers` and target recreation. On 2026-10-18,
  VS2022 Debug and Release engine/example builds passed. Debug and Release
  D3D12 120-frame runs each logged both successful resizes, passed compute and
  texture-array readbacks at frame 119, exited 0, and had empty stderr. Debug
  used the D3D12 layer; only the existing initial UAV-state warning appeared.
  Default-mode 120-frame D3D12 Debug/Release runs also passed both readbacks
  and exited 0. OpenGL Debug/Release and Vulkan shared/dedicated-queue
  Debug/Release default-mode 120-frame runs likewise passed both readbacks,
  exited 0, and had empty stderr; these runs did not exercise resize.
  Remaining: manually check a real D3D12 window size change, minimize/restore,
  visible result, and repeated-resize memory/debug output. The internal event
  test does not satisfy those interactive acceptance checks.
  User report (2026-10-18): the manual interactive checklist ran without a
  problem. This is user-observed evidence, separate from the agent's synthetic
  resize and backend runs; no manual log or memory samples were provided here.
  Keep the detailed debug-output/memory evidence open rather than treating
  this report as an automated verification of those measurements.

- [ ] Decide whether to implement or explicitly retire the scene component
  render helpers. `ImageRenderer::OnRender` contains only a commented-out draw,
  `MeshRenderer::OnRender` has its pipeline draw disabled, and
  `GraphicsTool::Init/RenderPoint/RenderLine` have empty bodies. Acceptance for
  implementation: a focused example visibly draws via each retained API on
  supported backends, including resize/replacement and clean shutdown; if an
  API is retired, remove or mark it unsupported in public headers and docs.
  Bounded implementation decision (2026-10-18): retain color-only quad
  `ImageRenderer` and material-backed `MeshRenderer` through the portable
  presentation command buffer; dispatch them from `GameObject::OnRender`.
  Require an explicit material rather than an application-relative hidden
  default shader. Reject textured images and `GraphicsTool` point/line calls
  explicitly until a backend-neutral lifetime/batching contract exists.
  Add `SceneRenderHelpers` as a new example with separate image/mesh shaders,
  a visible color quad and mesh triangle, negative unsupported checks, and
  bounded frame exit. Acceptance for this milestone: Debug/Release engine and
  example builds, 120-frame noninteractive runs on each available backend,
  useful logs and clean shutdown, plus documented manual visible and resize
  checks. Component and `GameObject` draw paths and explicit unsupported
  exceptions are implemented. New example project, shaders, render layer, and
  launch/expected-output guide added. VS2022 Debug engine/example build passed.
  A hidden D3D12 debug-layer run with both synthetic resizes completed 120
  frames and exited 0, logging both replacements and both successful buffer
  resizes; stderr was empty, and the mesh shader reflected all eight `Vertex`
  inputs at offsets through 88 bytes. This is recording/compilation evidence,
  not visual proof. Debug and Release default 120-frame runs on OpenGL and
  Vulkan also exited 0 with replacement logs and empty stderr; Release D3D12
  completed both synthetic resizes and exited 0. A follow-up synthetic
  Vulkan run repeatedly requested 960x540 without changing the native window,
  then stalled at frame 40; the agent stopped that run and narrowed the
  diagnostic to D3D12 with one-shot guards. After that fix, Debug and Release
  engine/example rebuilds passed. Six 120-frame noninteractive runs (D3D12,
  OpenGL, Vulkan in both configurations) exited 0 with empty stderr and the
  replacement/shutdown logs. D3D12 requested exactly two synthetic resizes
  per configuration; OpenGL/Vulkan ignored that diagnostic as documented.
  A scan found no validation errors in their captured stdout. Manual visible
  color/geometry, real resize/minimize/restore, and longer-run memory behavior
  remain unverified. Keep this item open until those checks are reported.
  User report (2026-10-18): the requested manual test had no problem. This is
  user-observed evidence for the visible/interactive checklist, without
  per-backend logs or long-run memory measurements. Keep the item open for
  those detailed backend and memory checks.
