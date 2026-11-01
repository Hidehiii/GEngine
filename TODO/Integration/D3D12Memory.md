## Build-time exact-version exclusion (2026-11-01)

- [x] Recovery guidance (user request): add concrete version-switch tips to
  rejected-build console/popup output and BuildAndRun, distinguishing system
  runtime servicing from Windows SDK retargeting and optional Agility integration.
  Keep exact-version rejection unchanged; do not install/update the host or add
  SDK selection as part of this documentation/message task. Acceptance: actionable
  update/recheck steps, official references, accurate current-project limitations,
  policy regression and actual no-popup failure output, architecture/API sync.
  Implemented console/popup shared recovery text and a linked BuildAndRun section
  covering OS update/Graphics Tools, recheck command, Visual Studio SDK retargeting
  and Agility's separate integration requirement. Official Microsoft installation,
  retargeting and Agility references reviewed. No host changes or SDK selection
  implemented. Policy regression passes; actual -NoPopup invocation reports the
  installed blocked version, displays all four tips and exits 1. Popup rendering
  was not re-inspected; it receives the same failure string. Engine rebuild is
  unnecessary for message/documentation-only changes; rejection logic unchanged.

- [x] User correction: remove the mandatory pinned Agility SDK and its build/
  deployment dependency. Final user direction: reject project builds and display
  a warning popup when installed x64 system SDKLayers is exactly 10.0.26100.4768.
  Integrate the same prebuild script into CMake and Premake; leave runtime device
  creation/public APIs/GPU scheduling unchanged. Missing optional Graphics Tools
  is not this defect; unreadable existing DLL metadata fails explicitly. Provide
  a popup-only suppression for CI/automated checks which never bypasses rejection.
  Acceptance: real Debug/Release build rejection on this host, exact-version and
  neighboring-version checks, popup attempt, generation/dependency review, no
  mandatory SDK package/deployment, documentation/diff review. Earlier successful
  rendering builds remain historical: the corrected policy intentionally rejects
  this host until its installed validation runtime changes.
  Earlier SDK pinning below is superseded history;
  its long-run results do not validate the revised exclusion policy.

  Implemented: shared CheckD3D12Runtime.ps1, CMake always-run dependency and
  Premake utility dependency (Debug/Release/Dist). Popup suppression never
  changes failure exit. The native WScript.Shell.Popup call completed with
  rejection exit 1 after its 10-second timeout and no popup-unavailable error;
  visual inspection was not performed. Runtime C++/headers and deployment
  configuration are restored; no SDK preparation script or package is required.
  Exact-version policy tests pass for the excluded version, adjacent revisions,
  other build/minor/major and Agility versions. CMake FrameGraphTriangle Debug/
  Release configure successfully then fail on the installed version before
  D3D12 compilation. Premake generation and generated utility Debug/Release
  builds reject the same version; 32-bit MSBuild correctly selects Sysnative.
  An initial solution invocation used an invalid nested target and was discarded;
  utility projects were then built directly with SolutionDir set. Initial
  Premake buildcommands generated no event; corrected to prebuildcommands and
  verified generated events/configurations and actual build failures.
  Logs: build/pipeline-lifecycle/build-FrameGraphTriangle-{Debug,Release}.log and
  build/d3d12-handle-fix/premake-check-{Debug,Release}.log. No successful current
  engine/runtime build is claimed on this intentionally excluded host. Other
  installed runtime versions and complete Premake engine builds remain untested.
  Historical attribution tools remain; the runner no longer assumes SDK 619.
  Existing tmp/ and backend selection are preserved.
  Final Premake hardening uses an absolute generated script path and a backend
  prebuild event as well as the solution utility, covering standalone backend
  project builds. The actual standalone Debug backend build rejects correctly
  without SolutionDir (premake-standalone.log); no D3D12 compilation occurred.
  Policy/runner syntax and git diff --check pass; unstaged review refresh queued.

## Handle-growth attribution and repair (2026-11-01, superseded SDK pinning)

- [x] User-authorized investigation/fix of current D3D12 handle/private-memory
  growth after P1-P3. Scope: reproduce matched workloads with real validation
  on/off, identify increasing native handle types and their creation/retention
  path, then repair the responsible engine synchronization/lifetime behavior.
  Keep default validation enabled; do not mask growth by disabling diagnostics
  or adding unexplained per-frame device waits. Use owned child processes and
  bounded programmatic diagnostics; no desktop automation or machine-wide
  tracing configuration. Existing tmp/ is preserved.
  Acceptance: concrete failing path and native attribution, focused regression
  that fails before the repair and passes afterward, Debug/Release engine/example
  builds, matched frame-count handle/private-byte controls, clean native output,
  GPU readbacks and resize/replacement/shutdown coverage. Recheck affected P1-P3
  and all backends for any shared interface change. Keep unrelated driver/heap
  growth open if evidence does not attribute it. Update architecture/API,
  diagnostics/example guides and RenderingArchitecture with measured results.
  Initial source review: persistent completion fences/events are reused; the
  strongest historical growth needs validation on and occurs without replacement.
  First step is native handle-type/stack attribution, not another process-count
  trend alone. Public API/architecture semantics are unchanged by inspection.
  Reproduction: Release/no replacement/layer-on 30,000-frame owned-child run
  passed all GPU checks; native Event handles grew 7,046 at 2.5 s to 239,059 at
  56.5 s. A layer-off trace control stayed at 274->276 Events over 6,000 frames.
  Other handle categories were bounded. Per-process handle tracing was accepted
  but returned empty stack arrays on this host; this is type attribution only.
  Next isolate native Wait/Signal/SetEventOnCompletion/Execute calls with a small
  device-only probe, preserving validation and checking counts after queue idle.
  Attribution: native pending-wait/layer-on 30,000 calls grow 389->30,390 handles
  after idle and still leave 30,227 after device release. Completed waits,
  Signal, CPU event waits, completed list reuse/reset controls are flat; layer-off
  pending waits stay flat. An early empty-list probe resubmitted before completion
  was invalid and discarded; the corrected control waits before reuse. System
  D3D12Core/SDKLayers are 10.0.26100.4768. A Microsoft-signed Agility 1.619.5
  matched pending-wait probe stays 391->395 after idle and 229 after device
  release, with validation enabled. This attributes the unbounded Events to the
  system SDK layer's pending Queue::Wait handling, not engine-created events.
  Repair scope: pin and prepare official Agility 1.619.5 (SDK 619) with SHA256
  verification, load it through a backend-owned independent device factory,
  configure full debug/synchronized-queue validation on that factory, and deploy
  the paired runtime/layers plus licenses beside each native executable through
  CMake/Premake/export. Do not change queue ordering or add CPU frame waits.
  Missing/mismatched runtime fails explicitly; no silent fallback to leaking
  system validation. Verify loaded SDK/validation flags and actual module paths,
  matched engine controls, missing-runtime failure and export/cold launch.
  Deployment checks: isolated missing-runtime copy fails before writing output;
  a copied missing-runtime executable exits nonzero but the uncaught startup
  exception drops its message on MSVC. Log the actionable missing-runtime message
  before throwing so failure is explicit in native logs. A non-ASCII executable/
  runtime path passed 120 frames, validation and all three readbacks. SDK 619
  factory flags confirm full debug and synchronized queue validation.
  Bootstrap robustness check: a native/Python-launched Windows PowerShell child
  lacks the Get-FileHash script cmdlet under the host's inherited module path.
  Use disposable .NET SHA256/FileStream directly, preserving the same pinned
  digest and avoiding global environment/module changes; retest valid/invalid
  archives in that exact launch path. The first missing-runtime launch fixture
  also omitted required config keys and failed before device creation; discard
  that attempt. Full-config rerun reaches the missing-SDK check, logs actionable
  diagnostics and exits nonzero without fallback. No normal-startup behavior
  changes beyond the explicit error log.
  Verification complete (2026-11-01): the isolated regression fails on the
  pre-fix Release binary at 6,000 frames (Event range 21,149 > budget 256),
  after all GPU checks passed. Final Debug/Release 30,000-frame layer-on runs
  pass with Event ranges 290..298 and 292..313 respectively, SDK=619/debug=true/
  synchronizedQueueValidation=true, paired app-local module paths, three GPU
  readbacks, normal exit and no native warnings/errors. Private bytes in sampled
  steady windows: Debug 174575616->175243264; Release 174182400->174370816.
  Logs/JSON: build/d3d12-handle-fix/baseline-regression, fixed-Debug-on,
  fixed-Release-on. Probe logs: probe-system.log and probe-agility.log; matched
  30,000 pending waits leave 30,001 versus four extra handles after idle, and
  30,228 versus 229 total handles after device release. No private DLL stack
  attribution is claimed; the failing native API/validation-runtime combination
  is directly isolated, and the paired runtime update removes it.
  Release matched 30,000-frame controls (matched-Release):
  layer off/no replacement: handles 707->707, private 162000896->162193408;
  layer off/replacement: 704->704, 162193408->163078144;
  layer on/no replacement: 748->763, 176721920->177250304;
  layer on/replacement: 752->745, 175263744->175751168.
  All four pass readbacks, shutdown, native validation and bounded retirement.
  Earlier historical layer-on controls reached 271,487/281,367 handles at the
  same frame count. Debug P3 6,000 frames also passes 349 replacements/403 probes
  with handles 748->748 and private 188461056->189296640 (post-stress-Debug).
  D3D12 P1/P2 Debug/Release and P3 Release acceptance pass (p1-/p2-/p3-*);
  MSAA/MRT/depth/LOAD/resize regression passes all four backend variants in
  Debug/Release (msaa-*). Final engine DLL hashes match both examples in both
  configurations; all deployed DLL/license bytes match the prepared package.
  CMake VS2022 native engine/examples and real Ninja export builds pass;
  Premake VS2022 generation and emitted SDK deployment commands pass inspection.
  Exported Runtime/D3D12 includes all five files; packaged replacement/no-
  replacement/discard/synthetic-resize/eight-second stress pass with clean
  diagnostics (package-check, handles 751..776). A cache/log-free package copy
  launched through the root launcher from an empty cwd passes SDK/full-validation
  and readback checks without extracting Assets (cold-package/cold-launch.log).
  Non-ASCII executable paths, missing-runtime startup, mismatched system-layer/
  app-Core startup and missing-runtime deployment checks pass; malformed hash
  is rejected before extraction and the valid offline archive prepares in a
  native-launched Windows PowerShell child. Full-config missing-runtime failure
  intentionally exits nonzero; there is no system fallback. All owned processes
  have finished. No device-wide frame waits, disabled validation, vendor changes,
  OS runtime replacement, global environment/profiler changes or desktop control.
  Remaining limits: other OS/driver combinations, full Premake/Mono builds,
  interactive window checks, injected device loss/OOM and indefinite runs are
  untested. Historical unrelated driver/heap groups remain unattributed below;
  this does not mean the reproduced Event leak is still open on the pinned SDK.
  No commit/push requested for this fix; preexisting tmp/ and backend config stay
  untouched. Link/diff review and unstaged panel refresh are handoff checks.
  Handoff checks passed: 72 relative documentation links, diagnostic-runner syntax,
  final DLL/runtime/license identity checks and git diff --check. Intended source/
  deployment diff reviewed; each Premake consumer has exactly one SDK copy hook.
  Unstaged review refresh requested (queued by the app); staging remains empty.

## Current-source matched-frame follow-up (2026-10-31)

Bounded diagnostic milestone completed under
[Phase4](../Rendering/Phase4.md#foundation-closeout-milestone-2026-10-31).
Tests/RenderingFoundation.py ran current-source Release FrameGraphTriangle with
the real debug layer off/on and replacement off/on for 30,000 frames each.
All four exited 0 with three GPU readbacks, normal shutdown and no native
validation errors. Configurations/assets were isolated; no desktop automation,
elevated tracing or system setting changes were used.

After 3 seconds and before the final teardown second, layer-off handles were
706->706 / 703->707 and private bytes 146.01->146.30 / 143.94->145.23 MiB
(no replacement / replacement). Layer-on handles were 14819->271487 /
15217->281367 and private bytes 164.45->180.43 / 165.26->181.63 MiB.
Elapsed durations differed (21.5/19.7 s off, 56.1/54.1 s on); these are matched
frame counts, not matched time samples. Pending submissions sampled at most 15;
deferred releases and stored/discarded debug messages were 0. Replacement
pipeline traces logged 1000 created, 3 live. Context-owned native resources/PSOs
were still present at reporting time and differed with replacement (11/13
resources, 5/8 PSOs), so this is not a post-device leak report. Growth without
replacement is reproduced; native handle/allocation attribution remains open.
Final process-exit samples can contain zero handles/released working memory and
are excluded from trend summaries. Raw logs/CSV remain in
build/foundation/lifetime-Release/. Earlier stack attribution below is historical
evidence, not a new current-source profiler capture.

## Open diagnosis and historical evidence

The current native Event-handle leak is resolved above. The older heap traces
below remain historical evidence about additional driver/CPU bookkeeping; they
do not establish continuing growth with the pinned runtime or assign every heap
allocation to the Event defect.

- [ ] Isolate D3D12 Debug Layer private-byte growth. It rose by about 17-19 MB
  over 110 seconds with and without resource replacement, while the layer-off
  run remained within about 0.6 MB. Acceptance: use matched frame-count and
  time runs, inspect live-object/debug-layer diagnostics and driver allocation
  behavior, then distinguish engine-owned growth from diagnostic or driver
  overhead. Do not label the current process-level observation a leak.
  Additional observation (2026-10-28, during the UAV synchronization fix): kernel
  handle count grows with validation enabled in both the old Debug development
  binary (5,695 -> 20,608 sampled after warm-up during a 6-second run, 4,020 total
  frames) and the fixed Debug binary (6,575 -> 24,525, 3,926 total frames).
  Layer-off controls stayed near 519/522 handles before shutdown. The fixed
  10-second Release validation run had pending submissions 1-5 at the final
  30k-60k submission samples, deferred releases 0 and stored/discarded messages 0,
  yet handles still increased. This does not attribute ownership to the driver
  or validation runtime and is not a new per-frame fence allocation in the fix.
  Keep this separate from the now-verified UAV ordering repair. Remaining:
  use matched frame-count controls and native handle-type/allocation attribution;
  verify release after GPU idle/shutdown before assigning a leak. Logs/CSV are
  build/d3d12-handles-*.log and build/d3d12-uav-*-regression/*-memory.csv.
  Next bounded diagnosis: run 60,000-frame D3D12 Debug comparisons with the
  layer on/off from isolated configuration copies, sample private bytes at
  start/end, and enable the existing pipeline lifetime trace. Inspect the
  bounded info-queue and native resource-retirement paths, plus available GPU
  memory counters. Acceptance for this step: attribute only what the evidence
  supports, keep unresolved ownership open, and do not use desktop automation.
  The 60,000-frame comparison passed on 2026-10-11: both modes exited 0 and
  passed the two GPU readbacks; at 2,000 created pipelines the trace reported
  only two live pipelines. With the layer off, private bytes stayed near
  173.5 MB and GPU dedicated/shared usage stayed near 37.7/61.4 MB. With it
  on, private bytes rose from 191.9 MB at 10 seconds to 207.8 MB at 110
  seconds while GPU dedicated/shared usage stayed near 38.8/66.4 MB. The
  existing logger polls and clears an info queue limited to 1,024 messages.
  This excludes a growing pipeline count or sampled GPU allocation as the
  observed process-memory cause, but does not separate engine logger behavior
  from validation-layer CPU bookkeeping. Next isolate the logger with an
  explicit opt-out for diagnostic runs; default diagnostics must stay on.
  Acceptance: the opt-out affects only the polling logger, not Debug Layer
  device creation or the bounded info queue; Debug engine/example build and a
  matched 60,000-frame opt-out run pass, and its memory slope determines
  whether further validation-layer/driver investigation remains necessary.
  Implemented: `GENGINE_D3D12_INFO_QUEUE_LOGGER=0` skips only the polling
  thread; validation setup, the 1,024-message queue limit, and filter still
  run. The VS2022 Debug engine/example build passed in an isolated child
  environment after a direct invocation hit the host's duplicate PATH/Path
  MSBuild error. The logger-off, layer-on 60,000-frame run exited 0, passed
  both readbacks, and again reported two live pipelines at 2,000 created.
  Private bytes rose from 194060288 at 10 s to 209833984 at 114 s, while
  sampled GPU dedicated/shared usage stayed at 38785024/66408448 bytes.
  This closely matches the logger-on slope, so the polling thread is not the
  primary cause. The growth is associated with enabling D3D12 validation or
  its driver interaction, but exact CPU allocation ownership is unresolved;
  keep this TODO open for a native live-object/allocation attribution pass
  (for example, compare debug-device live-object summaries and a CPU allocation
  trace with validation on/off). VS2022 Release engine/example also built in
  the isolated environment; a 120-frame Release D3D12 run passed both
  readbacks, exited 0, and had empty stderr. The logger opt-out is Debug-only.
  Next bounded step: add an opt-in, sparse trace of tracked submissions,
  pending submissions, deferred releases, and stored/discarded validation
  messages. Run the Debug Layer-enabled example for 60,000 frames from an
  isolated configuration without controlling the desktop; compare the trace
  with private bytes and GPU process counters. Acceptance: Debug/Release
  engine and example build, both GPU readbacks and normal shutdown pass, and
  any remaining need for native CPU allocation attribution stays explicit.
  Completed 2026-10-11: the opt-in trace logs every 10,000 submissions in
  Debug. VS2022 Debug and Release engine/example builds passed. A Debug Layer-on
  60,000-frame run from an isolated D3D12 configuration exited 0 after about
  125 seconds, passed both frame-119 GPU readbacks, and had empty stderr.
  Between 10,000 and 360,000 submissions, pending submissions sampled at
  1-10, deferred releases at 0, and stored/discarded info-queue messages at
  0. GPU dedicated/shared usage stayed around 38,785,024/66,408,448 bytes;
  private bytes rose from 193,114,112 at 16 seconds to 207,142,912 at 115
  seconds. The one known initial buffer-state warning remains. Thus sampled
  engine retirement queues and the bounded message store did not grow with
  process private bytes. This still does not identify CPU allocations inside
  the D3D12 validation runtime, driver, or other engine components. Remaining:
  use native CPU allocation stacks and a correctly timed live-object report
  with layer on/off before assigning ownership or closing this TODO.
  Bounded follow-up (2026-10-18): add an opt-in D3D12 Debug Layer live-object
  summary at context shutdown, after the application layer and renderer have
  released their objects and GPU work has completed. Compare otherwise matched
  short and long FrameGraphTriangle runs with the layer enabled, plus a
  layer-disabled process-memory control. Acceptance: the report is disabled by
  default, both Debug and Release still build, runs exit normally with GPU
  readbacks, and the report identifies whether externally referenced D3D12
  object counts rise across frame counts. Context-owned objects are still
  alive at this point, so equal counts are useful but not a final leak audit.
  CPU allocation-stack attribution remains separate; WPR heap tracing needs
  elevated, machine-level tracing configuration and will not be enabled as a
  routine repository diagnostic.
  Implemented the default-off Debug-only `GENGINE_D3D12_LIVE_OBJECTS=1` report
  in `D3D12Context::Uninit`; it stops the polling logger before requesting a
  summary and copies available messages to the engine log. VS2022 Debug and
  Release engine/FrameGraphTriangle builds passed. A 120-frame Debug Layer-on
  run and a 60,000-frame Debug Layer-on run both exited 0, passed both GPU
  readbacks, and had empty stderr. Each shutdown report contained the same
  12 messages and identical object categories/counts after normalizing the
  device address: 3 queues, 1 swap chain, 26 fences, 27 allocators, 25 command
  lists, 11 resources, 8 descriptor heaps, 1 root signature, and 5 pipeline
  states. The context still holds these objects at report time. In the long
  run, private bytes rose from 193,826,816 at 10 s to 209,403,904 at 70 s;
  the only Debug Layer warning was the already-known initial UAV buffer state.
  A layer-off 60,000-frame control exited 0 and stayed near 171 MB from 10-40 s.
  An 80-second layer-off control exited 0 after 102,423 frames and stayed near
  169 MB from 10-80 s. Both controls passed readbacks and had empty stderr.
  A 120-frame Release run also passed readbacks and normal shutdown; the
  Debug-only live-object switch produced no report there. Thus no increasing
  externally referenced D3D12 object count was observed at this reporting
  point, while process growth remains associated with validation being on.
  This does not assign CPU allocation ownership or prove leak freedom. Native
  heap allocation stacks require separate elevated tracing; keep the parent
  TODO open pending an approved, correctly scoped profiler capture.
  User-authorized WPR follow-up (2026-10-18): capture heap allocation stacks
  for the existing Debug FrameGraphTriangle executable with the D3D12 Debug
  Layer on, and compare with a layer-off control. Before starting, confirm no
  other WPR recording and that per-process heap tracing is disabled. Use
  bounded isolated runs, retain only evidence needed to attribute growth,
  and stop the recording and disable the per-process tracing flag afterward.
  Acceptance: identify the largest surviving allocation stacks/modules or
  report the exact permission/tool limitation; do not call private-byte growth
  a leak without a matching allocation trace. No desktop UI automation.
  Attempted from the current non-administrator session: `wpr -status` reported
  no recording and `wpr -HeapTracingConfig FrameGraphTriangle.exe` reported
  disabled. `wpr -HeapTracingConfig FrameGraphTriangle.exe enable` returned
  `0x80070005 Access is denied`, including when run through the approved
  command escalation. A second query confirmed the flag stayed disabled and
  WPR still was not recording. No trace was captured and no system profiler
  setting was changed. Remaining: an administrator must capture a bounded
  Heap-profile ETL with reliable stop/disable cleanup, then provide its path
  for stack attribution. Keep this item open.
  User subsequently authorized an agent-started elevated capture. Next
  attempt: launch one UAC-approved, bounded PowerShell helper (without desktop
  automation) that records layer-on and layer-off runs in isolated working
  directories, then stops each WPR session and disables the process tracing
  flag even on failure. Acceptance: two readable ETLs, matching run logs and
  memory samples, verified WPR idle/flag disabled afterward, and stack-level
  attribution or a precise tooling/permission blocker.
  Elevated capture result (2026-10-18): the user approved UAC. The first
  helper run saved a 5,295,308,800-byte layer-on ETL but reported an empty
  PowerShell `Start-Process` exit-code property, so it stopped before the
  control; its cleanup disabled heap tracing. The helper was corrected to
  require both GPU readbacks, normal-shutdown log, and empty stderr when the
  exit code is unavailable. A second elevated run reused that ETL and saved
  a 3,536,846,848-byte layer-off ETL. The on/off examples ran 60-second
  limits, reaching 16,973/37,203 frames respectively. Both logged the two
  frame-119 readbacks and normal shutdown with empty stderr. Afterward,
  `wpr -status` reported idle and `wpr -HeapTracingConfig
  FrameGraphTriangle.exe` reported disabled. ETLs and extracted stack reports
  remain in `tmp/d3d12-wpr-20261018/` for inspection; they are untracked and
  contain system trace metadata.
  Private bytes under WPR rose from 184,946,688 at 10 s to 188,690,432 at
  55 s with the layer on; the layer-off control stayed around 169,242,624
  through 55 s. `xperf -a heap` identified the target PIDs and reported
  end-of-process outstanding heap allocations of 2,963.0 KiB on versus
  1,582.2 KiB off. In the 10-60 s event window, the outstanding-at-window-end
  heap records were 3,393.5 KiB on versus 275.8 KiB off. The on trace has
  a 1,964.3-KiB stack through `nvwgf2umx.dll`, plus a 169.9-KiB stack
  through `nvwgf2umx.dll`, `D3D12Core.dll`, and `d3d12SDKLayers.dll` from
  `D3D12CommandBuffer::ResetRecording`; the latter records 10,876 small
  allocations in that window. A 525.4-KiB group has only `ntdll` frames
  and cannot be assigned to a module. The off trace lacks the SDK
  layer path and has only 275.8 KiB outstanding in that interval. Together
  with the identical short/long live-object counts, this points to validation
  runtime/driver-side CPU bookkeeping and allocation churn during command
  recording, not accumulating engine-owned D3D12 object handles. It does not
  establish a leak: frame rates differ, range-end outstanding allocations
  may be freed during later teardown, and the `ntdll`-only group is
  unattributed. Keep this TODO open for symbolized allocation-stack or
  validation-mode isolation before assigning all private-byte growth.
  Bounded validation-state fix (completed, 2026-10-18): correct the D3D12
  storage-buffer initialization path that requests UAV initial state even
  though D3D12 creates buffers in COMMON, then verify that the graph's
  imported ShaderWrite state and SetData/ReadData transitions are truthful.
  Acceptance: initialize in COMMON and explicitly transition to UAV before
  exposing the buffer; Debug and Release engine/example builds pass; a
  Debug Layer-on FrameGraphTriangle run passes both GPU readbacks without
  the initial-state warning; compare its private-byte trend without claiming
  this one-time warning caused the ongoing driver/validation allocation
  growth. Remaining driver-side ownership stays open.
  Implementation/verification: `D3D12StorageBuffer` now creates its buffer
  in COMMON and completes a one-time COMMON-to-UAV transition before exposing
  it. VS2022 Debug and Release engine/FrameGraphTriangle builds passed. A
  120-frame Debug Layer-on run and a 120-frame Release run passed both GPU
  readbacks, exited 0, and had empty stderr; the original initial-state
