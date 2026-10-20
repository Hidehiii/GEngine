# Runtime logging

- [ ] Add an optional in-application log viewer as a separate follow-up. Scope:
  design the viewer only after consumers need live filtering while the engine
  runs; it must consume the same logging stream without adding unbounded
  per-frame memory or making file logging depend on the UI. Acceptance: a
  focused example demonstrates live records, filtering, bounded retention, and
  normal shutdown on available backends. Status: plan only; no viewer API or
  UI work is authorized or implemented by this registration.

- [x] Add a compile-time macro to control local log-file saving without
  disabling console logging. Scope: `GE_ENABLE_LOCAL_LOG_SAVE` defaults to 1
  in the public log header; engine builds with 0 skip file/directory creation
  but keep core/client console logging and runtime level filtering. Existing
  environment overrides apply only when saving is enabled. Acceptance:
  default and disabled builds succeed in Debug and Release; an example run
  with 0 produces console records but no log file; default behavior remains
  unchanged. Update architecture, API, and example documentation. Status:
  registered 2026-10-18; macro and code path implemented, architecture/API and
  example documentation updated. Added a focused `Tests/LocalLogSave.cpp`
  regression that compiles the engine log implementation under both macro
  values. Verification (2026-10-18): default Debug and Release engine/example
  builds passed. The focused test compiled and ran the same `Log.cpp` under
  Debug and Release with both `GE_ENABLE_LOCAL_LOG_SAVE=0` and `=1`; all four
  runs exited 0 and kept core/application console records. Only the enabled
  runs created `Logs/GEngine.log`. A hidden 120-frame Release D3D12 example
  run with the default macro exited 0, wrote the file, and logged frame-119
  GPU readback and normal shutdown. The disabled variant was verified by the
  focused regression, not a full backend/example rebuild. User report
  (2026-10-18): the requested follow-up test is done. This closes the item on
  user acceptance; no per-configuration build logs, console captures, or
  file-absence measurements were supplied to the agent, so the full macro-off
  example run remains user-reported rather than agent-verified. OpenGL and
  Vulkan were not rerun for this macro change.

- [x] Make engine and application logs useful while an application is running,
  without depending on Visual Studio's Output window.
  Baseline evidence (before this implementation, 2026-10-18): `Log::Init` creates `GEngine` and `APP`
  spdlog stdout-color loggers at trace level. It runs before the runtime
  changes to `RuntimeSpecification::WorkingDirectory`. There is no engine-owned
  persistent file sink, bounded retention policy, or built-in live log viewer.
  The existing `GE_CORE_*` and `GE_*` macros already distinguish engine and
  application messages; this item does not assume that logging is absent.

  Phased plan:
  1. Define a runtime log destination after resolving the application working
     directory, retaining stdout output. Document how early startup failures
     are reported before the file sink is ready.
  2. Attach a shared, thread-safe rotating file sink to both existing loggers.
     Give each record a timestamp, level, logger name, and message; keep disk
     use bounded with documented file size/count defaults. Preserve the
     existing public macros and avoid logging every frame by default.
  3. Expose a small runtime configuration for minimum level and log location,
     with safe defaults in Debug and Release. Flush errors promptly and flush
     and close the file at normal shutdown. Define behavior when the log path
     is unwritable without losing console diagnostics.
  4. Add a focused example/regression: emit core and application messages
     during a timed run, confirm they appear in the file before process exit,
     exercise rotation and concurrent writers, and verify normal shutdown.
     Treat in-application ImGui viewing/filtering as a separate optional
     follow-up using the same log stream, not a prerequisite for file logging.

  Acceptance for the first implementation milestone: Debug and Release engine
  and example builds; a running example exposes fresh core/application records
  in a bounded file without changing existing stdout behavior; levels and
  timestamps are present; unwritable-path fallback and shutdown are tested;
  no unbounded per-frame output or resource leak is introduced. Record the
  exact path, defaults, failure indicators, and example commands in the API,
  architecture, and example documentation. Implementation status and test
  evidence are recorded below.

  Bounded implementation choice (2026-10-18): keep the current stdout loggers
  alive through early startup, then attach one shared synchronous rotating file
  sink after `WorkingDirectory` is applied. Default file: `Logs/GEngine.log`
  relative to that directory, 5 MiB per file, three rotated files. Optional
  `GENGINE_LOG_FILE`, `GENGINE_LOG_LEVEL`, `GENGINE_LOG_MAX_BYTES`, and
  `GENGINE_LOG_MAX_FILES` environment overrides support diagnostics and a
  small rotation regression. Invalid values warn and use defaults; file-open
  failure warns and retains stdout. Info and higher levels flush promptly;
  normal shutdown flushes both loggers. A separate opt-in example workload
  exercises mid-run visibility and concurrent writes without making the
  normal frame loop noisy.

  Implementation progress (2026-10-18): `EngineRuntime` retains stdout during
  early startup, attaches a rotating sink after resolving the working
  directory, and flushes/closes logging after window teardown. Both core and
  client loggers share the sink. The example's
  `GENGINE_EXAMPLE_LOG_STRESS=1` path emits from two worker threads at frame 30.
  API, architecture, and example documentation now describe the implemented
  behavior. Debug engine/example build passed. A hidden, noninteractive Debug
  D3D12 run showed timestamped core/application records in the file while the
  process was alive, both workers' records, 1 KiB rotation into two retained
  files, the frame-119 GPU readbacks, and a normal-shutdown request. A second
  120-frame Debug run used an existing directory as `GENGINE_LOG_FILE` and
  invalid level/size values: stdout reported all fallbacks and the process
  exited 0. Release engine/example build passed; a 120-frame Release D3D12
  run exited 0 with the log stress markers, compute readback, and shutdown
  request in the file. Release OpenGL 4.6 and Vulkan 1.4.312 also exited 0
  after 120 frames with runtime logs and compute readback. No stderr messages
  were observed in these Release runs. This completes the first file-logging
  milestone, but does not establish visible presentation or long-run memory
  stability. A live ImGui log viewer remains optional future work, not part of
  this milestone.
