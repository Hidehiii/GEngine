# Cross-computer integration (2026-09-19)

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

## Fresh merged-version verification

Debug, VS2022, 2026-09-19: OpenGL, Vulkan shared queues, Vulkan dedicated queues,
and D3D12 each completed 120 frames, passed the frame-119 compute readback and
exited normally with code 0. D3D12 debug layer was enabled for this check.
Vulkan dedicated queues selected graphics=0, compute=2, transfer=5. No Vulkan
validation errors or D3D12 errors were found in captured output. Known warnings:
OpenGL shader-state recompilation / one-time readback transfer, and D3D12
storage-buffer initial UAV state being ignored in favor of COMMON.

Debug replacement stress ran all three APIs for 65 seconds concurrently,
including material/buffer/pipeline/render-pass replacement every 30 frames.
Vulkan used dedicated queues. D3D12 debug layer was disabled only for the
normal-runtime memory baseline (the separate diagnostic run above enabled it).

| Backend | Frames | Private bytes at 10 s | Private bytes at 60 s | Exit |
| --- | ---: | ---: | ---: | ---: |
| OpenGL | 193170 | 171413504 | 171253760 | 0 |
| Vulkan | 39271 | 415576064 | 415502336 | 0 |
| D3D12 | 50500 | 166256640 | 166567936 | 0 |

Samples were taken every 10 seconds. These bounded observations showed no
sustained allocation growth; they are not proof of indefinite leak freedom.
All three readbacks passed, stderr was empty, and no validation errors were
found. Logs remain locally under `.integration/runs-20260919/<backend>`.

Release, VS2022, 2026-09-20: OpenGL, Vulkan shared queues, Vulkan dedicated
queues and D3D12 each completed 120 frames, passed compute readback and exited
with code 0. All captured stderr files were empty. Release does not provide the
Debug validation-layer coverage described above.

Remaining checks: manual visible output/resize/minimize,
extended D3D12 validation-layer memory behavior, and VS2026 Debug compatibility.
The remote branch's VS2026 vendored spdlog/fmt Debug failure is not resolved by
the VS2022 build. No claim is made for unexercised examples/resource variants.
