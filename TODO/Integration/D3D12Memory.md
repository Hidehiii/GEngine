- [ ] Isolate D3D12 Debug Layer private-byte growth. It rose by about 17-19 MB
  over 110 seconds with and without resource replacement, while the layer-off
  run remained within about 0.6 MB. Acceptance: use matched frame-count and
  time runs, inspect live-object/debug-layer diagnostics and driver allocation
  behavior, then distinguish engine-owned growth from diagnostic or driver
  overhead. Do not label the current process-level observation a leak.
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
