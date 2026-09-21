# Workflow tracking

- [x] Establish repository engineering standards in `AGENTS.md` and link them
  from the documentation index.
- [x] Require every new work item to be registered in `TODO/` before implementation,
  then retain its scope, acceptance criteria, implementation and verification
  status through completion. Acceptance: the rule is documented in `AGENTS.md`.
  Verified by reviewing the written rule and this tracking entry.

## Build configuration reproducibility

- [x] Use the repository's vendored Vulkan SDK by default while retaining
      `VULKAN_SDK` as an optional override. Scope: update `Dependencies.lua`,
      document the default and override, then regenerate projects and build the
      Release engine. Acceptance: generation and compilation do not require a
      machine-wide Vulkan SDK when the vendored SDK is present, and an explicit
      `VULKAN_SDK` still selects the alternate SDK.
      Implemented: `Dependencies.lua` uses a non-empty `VULKAN_SDK` override
      when present and otherwise resolves the vendored SDK relative to the
      workspace. Build documentation describes both paths.
      Verified: VS2026 generation without a machine-wide `VULKAN_SDK` emits
      vendored include/library paths, and isolated Release builds of GEngine,
      its dependencies, and FrameGraphTriangle succeed. A child-process
      `VULKAN_SDK` override also selects that SDK during generation.

## Visual Studio 2026 project generation

- [ ] Add a VS2026 option to `Scripts/GenerateProjects.bat`, preserving the
  existing VS2022/VS2019 choices. Allow an external Premake executable without
  replacing vendored binaries, and resolve the repository relative to the script.
  Acceptance: `vs2026` is dispatched explicitly; unsupported Premake versions
  fail with actionable guidance; generation failures return a nonzero status;
  usage documentation names the real script and explains the prerequisite.
  Evidence before implementation: the bundled Premake reports `5.0.0-beta2`;
  its `--help` lists `vs2022` and `vs2019`, but not `vs2026`.
  Implemented: menu option `3`, noninteractive `vs2026` argument, `PREMAKE_EXE`
  override, script-relative repository resolution, explicit unsupported-action
  diagnostics and exit-code propagation. Updated build instructions and fixed
  the stale `GenerateProject.bat` links.
  Verified: bundled-Premake VS2026 rejection, invalid argument and missing
  executable each returned exit code 1. Reviewed the diff and documentation
  targets; `git diff --check` passed.
  Follow-up generation evidence: Premake beta8 emits `GEngine.slnx` for the
  `vs2026` action; generated solutions are ignored and the build guide names
  the solution format selected by each Visual Studio version.
  Initial validation limitation: VS2026 generation required an external
  Premake (resolved by the bundled upgrade below).
  VS2019/VS2022 regeneration was skipped after detecting MSBuild
  PID 29284 on 2026-09-18, to avoid modifying projects during another build.
  Follow-up build evidence: isolated VS2026 Release builds of GEngine and
  FrameGraphTriangle succeeded on 2026-09-18 using a clean child developer
  environment and separate object/output directories. The shared-output Debug
  build, VS2019/VS2022 regeneration, and backend runtime checks remain
  unverified. The Release D3D12 run is blocked at frame 90 by the repeated
  buffer/pipeline replacement failure recorded in the rendering TODO.

### Bundled Premake upgrade (user-authorized follow-up)

- [x] Download the official Premake 5.0.0-beta8 Windows release and replace
  `vender/bin/premake/premake5.exe`. Acceptance: verify the release asset
  checksum when provided, executable version and `vs2026` action; update usage
  documentation so VS2026 no longer requires an external executable.
  Before implementation: MSBuild PID 29284 and cl PID 19944 were active on
  2026-09-18. Do not regenerate shared project files while that build runs.
  Implemented: replaced only the executable from the official
  `premake/premake-core` release `v5.0.0-beta8`, asset
  `premake-5.0.0-beta8-windows.zip`; updated script diagnostics and usage docs.
  Verified: ZIP SHA256 matches the official GitHub release asset page:
  `e64ce2ed8778e0098f63674cca61fe33941b5f0c8d9a4afd651152bdea3758ab`.
  Installed executable matches the extracted release, SHA256:
  `2301e3e23ff3074cb83a5ea6103d68c7ea81dad56b786807c84b0643cddea31b`.
  `--version` reports `5.0.0-beta8`; `--help` lists `vs2019`, `vs2022` and
  `vs2026`. Documentation and diff review completed; `git diff --check` passed.
  Follow-up isolated Release engine/example generation and builds succeeded on
  2026-09-18. Debug, shared-output Release, VS2019/VS2022, and backend runtime
  compatibility remain unverified. Existing `flags` deprecation warnings appear
  under beta8; unrelated vendor scripts were not changed. The binary
  replacement milestone is complete, not full build compatibility verification.

## VS2026 Debug logging compatibility (2026-09-20)

- [x] Reproduce and fix the vendored spdlog/fmt checked_array_iterator build
  failure without editing vendor sources or disabling Debug iterator checks.
  Scope: inspect the installed STL and logging include boundary, add a focused
  public-header compile regression, then build GEngine/FrameGraphTriangle with
  VS2026 Debug and Release and run the existing three-backend GPU graph example.
  Acceptance: logging works without a precompiled header, Debug checks remain
  enabled, both configurations build, and backend readback/shutdown still pass.
  Recheck VS2022 header compatibility; record toolchain/runtime limitations.
  Initial evidence: the isolated no-PCH public-header probe passes under local
  VS2026 Insiders MSVC 14.50.35717 with Debug iterator level 2. The installed
  STL still defines checked_array_iterator; vendored fmt already includes
  <iterator>. The historical missing-type failure is not reproduced here.
  Add Tests/LoggingHeader.cpp and its launch documentation as a regression
  check, without unnecessary include shims, vendor edits or safety-check changes.
  Focused verification: VS2022 MSVC 14.44.35207 (v143) and VS2026 Insiders
  MSVC 14.50.35717 (v145) Debug/Release all compile
  and run the checked-in regression with exit code 0. Debug static_assert proves
  iterator level 2. Existing fmt C4996 deprecation warnings remain visible.
  Full VS2026 Debug rebuild passed (1596 warnings, zero errors). OpenGL, Vulkan
  shared queues, Vulkan dedicated queues (families 0/2/5), and D3D12 each ran
  120 frames, passed compute readback and exited with code 0. D3D12 debug layer
  was enabled.
  Follow-up 2026-09-21: after the previously active compiler finished, the
  VS2026 Release build of `GEngine` and `FrameGraphTriangle` completed with
  MSBuild 18.10.1 / MSVC 14.50.35717. The focused no-PCH Release logging-header
  regression also compiled and exited with code 0. An initial unconstrained
  MSBuild invocation created excessive node-reuse processes and was cancelled
  with user approval; the `/m:4` rerun produced updated Release artifacts.
  OpenGL, Vulkan shared queues, Vulkan dedicated queues, and D3D12 then each
  completed 120 frames, passed the frame-119 compute readback, exited with code
  0, and wrote empty stderr logs. Dedicated Vulkan selected graphics=0,
  compute=2, transfer=4. Release has no debug-layer validation coverage; manual
  visible-output, resize, and minimize checks remain untested.
