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

## Presentation and lifecycle follow-up (2026-10-11)

- [ ] Verify FrameGraphTriangle's visible triangle and color/motion changes on
  each available backend, then resize and minimize/restore the live window and
  close it normally. Acceptance: no crash or validation/debug error; rendering
  resumes at the new size and after restoration; the frame-119 compute and
  array-layer pixel readbacks pass. Record each backend and any UI limitation
  separately. Preserve the user's local Config.ini selection. The user asked
  for no desktop control, so this interactive portion remains unverified.
- [x] Run a bounded multi-minute replacement workload on each available
  backend, sample private bytes after warm-up, and compare with replacement
  disabled where useful. Acceptance: report sample times, backend/debug mode,
  normal exit and numerical readbacks, and any sustained growth without
  claiming indefinite leak freedom. Keep unrun backend cases open.
  On 2026-10-11, the existing VS2022 Debug executable ran each case for 120
  seconds without desktop input. Each exited 0 and passed both frame-119
  readbacks; stderr was empty. OpenGL used the user's selected API, while
  D3D12 and Vulkan used isolated configuration copies. Replacement was on
  except in the stated control. An earlier direct launch outside the example
  working directory aborted during startup; all measured runs used the
  documented working directory. Samples are private bytes, not GPU allocations:

  | Backend / mode | Frames | 10 s | 60 s | 120 s |
  | --- | ---: | ---: | ---: | ---: |
  | OpenGL 4.6 | 396350 | 173027328 | 172883968 | 172888064 |
  | D3D12 12.2, debug layer on | 60272 | 192450560 | 200187904 | 210235392 |
  | D3D12 12.2, debug layer off | 125314 | 170655744 | 170479616 | 171003904 |
  | D3D12 12.2, debug layer on, replacement off | 62511 | 192475136 | 202747904 | 211525632 |
  | Vulkan 1.4.312, dedicated queues | 76092 | 414896128 | 414720000 | 419622912 |

  Vulkan selected graphics=0, compute=2, transfer=5; no validation messages
  appeared. D3D12's debug layer logged one existing warning that an initial
  buffer UAV state is ignored in favor of COMMON, but no error. The Vulkan
  late increase and D3D12 debug-layer growth need more diagnosis; these
  bounded samples do not prove leak freedom or identify memory ownership.
  A normal shutdown is covered, but visible output, real resize, minimize,
  and restore are not. Release and Vulkan shared-queue long runs were not
  repeated in this milestone.
- [ ] Isolate D3D12 Debug Layer private-byte growth. It rose by about 17-19 MB
  over 110 seconds with and without resource replacement, while the layer-off
  run remained within about 0.6 MB. Acceptance: use matched frame-count and
  time runs, inspect live-object/debug-layer diagnostics and driver allocation
  behavior, then distinguish engine-owned growth from diagnostic or driver
  overhead. Do not label the current process-level observation a leak.
