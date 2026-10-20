# Cross-computer integration overview

- [x] Preserve local rendering work and integrate origin/master at 66866fea
  with local 82a231c5 and the uncommitted GPU graph milestone.
  Scope: save intended local changes on a backup branch; merge in an isolated
  worktree; reconcile backend lifetime, shader compilation, build configuration,
  examples and documentation without discarding either computer's fixes.
  Preserve the local FrameGraphTriangle backend configuration separately.
  Acceptance: both histories remain reachable, no conflict markers remain,
  Debug and Release engine/example builds pass, and OpenGL, Vulkan and D3D12
  complete the graph compute readback and shutdown checks. Record memory and
  validation output separately; do not inherit verification from either parent.
  Resize/manual visual checks must remain open unless actually exercised.
  No remote push is included in this task.

  Implementation: local work saved as 8cc72a72 on
  `codex/local-rendering-backup-20260919`; integration is on
  `codex/integrate-20260919` in `.integration/merge-20260919`.
  Eleven textual conflict files were reconciled. D3D12 keeps per-command
  completion/recording ownership and context-owned native-resource retirement.
  OpenGL uses the runtime initialization hook once, Vulkan SPIR-V translated
  to GLSL 450, and a cache fingerprint distinguishing this path from the remote
  direct-SPIR-V path. Component-count and null-index fixes are retained.
  Vulkan keeps null-safe deferred descriptor/layout/render-pass release and
  local cross-queue graph synchronization. The example now combines graph
  compute readback with material/buffer/pipeline/render-pass replacement.
  Release/Dist NDEBUG definitions are deduplicated; Premake beta8 and vendored
  SDK fallback are retained. Ignore `.integration` as local worktree storage.
  Verification: VS2022 generation succeeded with bundled beta8. The isolated
  Debug engine/example build passed (1596 warnings, zero errors); incremental
  recheck passed with zero warnings/errors. On 2026-09-20 the Release build was
  rechecked successfully with zero warnings/errors after the previous build
  session became unavailable; the earlier session's final result is not assumed.
  Semantic merge audit found duplicate VAO allocation in the OpenGL data-backed
  vertex-buffer constructor: both branches added creation at different lines.
  Keep one allocation so repeated replacement does not leak the overwritten VAO.
