# Workflow tracking

## Third-party dependency layout and submodules (planned)

- [ ] Rename `GEngine/vendor/` to `GEngine/Third-Party/` and make the resulting
  dependency tree a pinned Git submodule. Before implementation, identify or
  create the intended remote repository for that aggregate submodule and
  inventory each dependency's version, local patches, license, binary assets,
  and consumers. Preserve the nested `SPIRV-Reflect` submodule and its pinned
  commit in the new repository. Update `.gitmodules`, Premake, CMake, source
  includes, scripts, documentation, and
  ignore rules. Do not replace SDKs or binary-only packages with guessed
  upstream revisions or discard them to fit the new repository. Acceptance:
  the chosen submodule remote and pinned revision are documented; a fresh
  clone with recursive submodule initialization reproduces required files;
  Premake and CMake project generation resolve the new paths; Debug and Release
  GEngine plus FrameGraphTriangle builds and bounded backend runs pass; no
  local vendor patch or required runtime DLL is silently lost. Record the
  dependency provenance, verification evidence, and remaining work here when
  implementation is authorized.
  Initial inspection: `GEngine/vendor/` contains 23 top-level dependency
  directories and about 8,353 tracked paths; `SPIRV-Reflect` is already a
  submodule at commit `74cd6a01f274d6cc7fc9f14c9e3608047b09c5af` and is
  not initialized in this checkout. Current Premake and CMake definitions
  reference `GEngine/vendor/` directly. The aggregate submodule remote has
  not been specified. Status: plan only; no dependency move or submodule
  conversion has been made.

## CMake build option and build-script layout (2026-10-19)

- [ ] Unify Premake and CMake binary output paths. Rewrite the relevant build
  scripts/configuration so both methods place Windows x64 engine/application
  executables, DLLs, and required runtime DLLs at
  `bin/<Configuration>-windows-x86_64/<Target>/`, matching today's Premake
  layout. Inventory and align the output paths for static libraries, import
  libraries, and PDBs as well; do not assume only runnable files matter.
  Current evidence: Premake uses per-target paths, while CMake writes a flat
  `build/cmake-<version>/bin/<Configuration>/` and keeps static libraries
  under its build tree. Scope: define a consistent target-by-target layout for
  Debug/Release (and document CMake's existing Dist limitation), update
  post-build dependency copying and launch/debug working directories, and
  retain separate generator/object/PCH directories. Acceptance: configure and
  build GEngine and FrameGraphTriangle with each method in sequence; verify
  their final artifacts and copied DLLs match the path map, then run bounded GPU
  readbacks. Document that the two generators must not build the same
  configuration concurrently into the shared final path; check switching
  methods does not leave stale or mismatched binaries. C# ScriptCore's
  `GEngine-Editor/Resources/Scripts` output remains a documented exception
  unless the implementation explicitly adds CMake support. Record actual
  verification and any remaining target/configuration gaps here. Status:
  plan only; no build output path has changed yet.

- [x] Flatten the bundled Premake layout from
  `BuildScripts/Premake/bin/premake/premake5.exe` to
  `BuildScripts/Premake/premake5.exe`. Scope: move the unchanged binary, remove
  only the now-empty intermediate directories, and update the Premake launcher
  and build documentation. Acceptance: the binary hash is unchanged, VS2022
  generation succeeds from outside the repository, old nested folders are
  absent, and `git diff --check` passes. No engine API or runtime behavior
  changes; no engine rebuild is required.
  Implemented: moved the unchanged binary directly under
  `BuildScripts/Premake/`, removed the empty nested directories, and updated
  the launcher plus architecture, API, and build guides. Verification: SHA256
  stayed `2301e3e23ff3074cb83a5ea6103d68c7ea81dad56b786807c84b0643cddea31b`;
  VS2022 generation from `tmp/` returned 0, and the old `bin/` path is absent.

- [x] Add CMake as an alternative Windows C++ build generator while retaining
  Premake. Move the root `Scripts/` launchers to `BuildScripts/`, expose exactly
  `GenerateProjectsPremake.bat` and `GenerateProjectsCMake.bat`, move the root
  `vender/` Premake executable to `BuildScripts/Premake/`, and keep first-party
  CMake definitions under `BuildScripts/CMake/`. Scope includes the engine,
  vendored C++ libraries, and runnable C++ examples; the current C# ScriptCore
  generation may remain Premake-only if documented explicitly. Acceptance:
  the Premake launcher still generates VS2022 from any working directory;
  CMake configures VS2022 Debug/Release without a machine-wide Vulkan SDK,
  builds GEngine and FrameGraphTriangle in isolated outputs, and the example
  completes its bounded readback check. Update build instructions, links, and
  architecture/API build-function pages. Preserve unrelated worktree edits.
  Initial evidence: root `vender/` contains only `bin/premake/premake5.exe`;
  current Premake defines five vendored C++ libraries, GEngine, seven examples,
  Sandbox, Editor, and C# ScriptCore. CMake 3.29.1 is available on this host.
  First CMake Debug build exposed missing Unicode character-set definitions:
  wide-string Win32 calls resolved to ANSI entry points. Add `UNICODE` and
  `_UNICODE` to match the existing Visual Studio project behavior, then
  rebuild. The next link exposed the actual vendored OpenSSL layout at
  `GEngine/vendor/OpenSSL/lib/VC/x64/MD[d]`; use that path in CMake. The first
  Debug binary then built but exited with Windows DLL-not-found status;
  the post-build CMake script received quoted variable values and silently
  matched no runtime DLLs. Pass `-D` arguments without embedded value quotes
  and reject an empty DLL set.
  Implemented: moved the bundled executable and replaced the old launcher
  with Premake and CMake launchers. The CMake source tree defines the five
  vendored C++ libraries, GEngine, seven examples, Sandbox, and Editor, with
  isolated outputs and runtime DLL copying. The C# ScriptCore project and
  Premake's Dist configuration are not generated by this CMake option.
  Verification: both launchers generated VS2022 from a different working
  directory. CMake 3.29.1 configured with `VULKAN_SDK` cleared and generated
  vendored Vulkan include/library paths. VS2022 CMake Debug and Release built
  GEngine plus FrameGraphTriangle. Debug and Release D3D12, plus dedicated
  Vulkan with both exclusive-sharing options, each completed 120 frames with
  all three numeric readbacks, zero exit status, and empty stderr; no Vulkan
  validation messages were captured. Changed-document local links resolved,
  `git diff --check` passed, and the moved Premake binary retained its prior
  SHA256. Unbuilt CMake targets and VS2019 generation remain
  unverified; add focused checks if those workflows are needed.

- [x] Split the architecture and API overviews into function-focused files
  without losing the source-backed coverage added in the prior audit. Scope:
  retain `Architecture/Architecture.md` and `API/GraphicsAPI.md` as stable
  entry points, move runtime/scene/service/rendering architecture details and
  non-graphics API guidance to sibling documents, and update navigation and
  workflow links. Acceptance: each first-party subsystem remains represented,
  moved detail is not duplicated or dropped, all relative Markdown links and
  cited header/example paths resolve, and `git diff --check` passes. Preserve
  existing uncommitted edits; this documentation-only change needs content/link
  review, not an engine rebuild.
  Implemented: `Architecture/Architecture.md` is the subsystem index linking
  runtime, scene/services, and rendering/backend pages. `API/README.md` maps
  runtime, scene/physics, assets/services, graphics/compute, render graph, and
  shader authoring guides; `GraphicsAPI.md` remains the stable graphics path.
  Updated the documentation index, AGENTS, and Standard workflow to require
  corresponding function pages on both sides for future changes. Reviewed
  moved content against the prior guides: ownership/lifetime, examples,
  capabilities, resource rules, and explicit incomplete paths remain covered.
  Verified all relative Markdown links, absence of documentation trailing
  whitespace, and one architecture-index row for every first-party module
  directory. No build was run because this is documentation-only.
- [x] Expand the architecture and API guides to cover every first-party engine
  subsystem represented under `GEngine/include/GEngine` and `GEngine/src/GEngine`
  (plus backend/surface boundaries). Scope: inventory public headers, source
  ownership, initialization/frame/shutdown flow, key API entry points, examples,
  and current limitations; distinguish shipped behavior from open TODO plans.
  Acceptance: each engine subsystem has an architecture location and an API
  orientation or explicit internal-only designation; examples and signatures
  match current headers; cross-document links resolve; the diff and
  `git diff --check` pass. This documentation-only milestone needs link/content
  review rather than a build. Preserve existing uncommitted source and docs.
  Implemented: architecture subsystem map and cross-subsystem ownership/flow;
  engine-wide API entry-point map and usage/limitation notes across runtime,
  core/events, graphics/renderer/compute, scene/components, physics, scripting,
  assets/tools, math, ImGui, debug, utilities, surface, and backends. Updated
  the docs index and feature summary. Source inspection exposed three distinct
  gaps: scene fixed update does not step 3D physics, component render draws are
  disabled, and GraphicsTool drawing is empty; focused follow-ups are recorded
  in `SceneAndPhysics.md` and `RenderingArchitecture.md` without implementing
  them. Verified: all first-party module directories appear in the architecture
  map, named APIs were checked against public headers/implementation/example,
  and repository Markdown links resolve. This is content review, not a new
  engine build or runtime validation; retain backend-specific verification
  requirements for future code changes.
- [x] Place the remaining documentation topics in folders while keeping
  `Documentation/README.md` as the index. Scope: move StandardWorkflow and
  CrossComputerWorkflow to `Documentation/Workflow/`, BuildAndRun and Features
  to `Documentation/Guides/`, and D3D12Diagnostics to
  `Documentation/Diagnostics/`. Update repository-wide paths and the required
  architecture/API documentation status notes. Acceptance: every topic document
  is in a folder, previous local edits survive, all relative Markdown links
  resolve, no stale old paths remain, and `git diff --check` passes. This is a
  documentation-only relocation; no engine rebuild is required.
  Implemented: moved the five remaining topic files into Workflow, Guides,
  and Diagnostics while retaining the root documentation index. Updated AGENTS,
  the repository README, the index, intra-document links, and the architecture
  and API status notes; existing local code and configuration changes remain
  untouched. Verified: all relative Markdown links in the repository README,
  AGENTS, and every Documentation file resolve; old location references were
  reviewed, relocated files have no trailing whitespace, and `git diff --check`
  passed. No engine build was run for this documentation-only move.
- [x] Organize public API documentation under `Documentation/API/` and the
  architecture guide under `Documentation/Architecture/`. Scope: relocate
  GraphicsAPI, RenderGraph, and ShaderAuthoring without changing their content
  beyond necessary path/status notes; relocate Architecture, then update the
  documentation index, Standard workflow rule, AGENTS, and all affected links.
  Acceptance: both folders exist, previous uncommitted document edits survive,
  every affected relative Markdown link resolves, and `git diff --check` passes.
  Documentation-only relocation does not require an engine rebuild.
  Implemented: moved the three API guides and the architecture guide into
  separate folders without discarding their local edits. Updated AGENTS,
  Standard workflow, the documentation index, and links inside the moved files.
  Verified: checked all relative Markdown links in the moved documents and
  index, reviewed old-path references, and ran `git diff --check`. The example's
  local backend selection and unrelated code changes remain untouched.
- [x] Establish repository engineering standards in `AGENTS.md` and link them
  from the documentation index.
- [x] Require every new work item to be registered in `TODO/` before implementation,
  then retain its scope, acceptance criteria, implementation and verification
  status through completion. Acceptance: the rule is documented in `AGENTS.md`.
  Verified by reviewing the written rule and this tracking entry.
- [x] Make architecture and API documentation updates mandatory after every code
  or plan update. Scope: add a Standard workflow file, link it from the existing
  workflow and documentation index, and apply the rule to Architecture.md and
  GraphicsAPI.md for this workflow change. Acceptance: the rule requires both
  documents to be updated in the same work item, distinguishing planned from
  implemented behavior; if one surface is unaffected, record that explicitly
  rather than silently skipping it. Verify links, content, and `git diff --check`.
  Implemented: added `Documentation/Workflow/StandardWorkflow.md`, linked it from
  `AGENTS.md` and the documentation index, and updated both core documents for
  this workflow-only change. Verified: reviewed the rule and planned/implemented
  distinction, confirmed all referenced files exist, and `git diff --check`
  reported no whitespace errors. No engine build is required for this
  documentation-only change.

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

- [x] Add a VS2026 option to `Scripts/GenerateProjects.bat`, preserving the
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
