# Cross-computer integration (2026-09-19)

- [ ] Preserve local rendering work and integrate origin/master at 66866fea
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
