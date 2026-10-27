  Keep these as package-managed or engine-owned payloads rather than direct
  submodule replacements: `dxc` (binary release from
  `https://github.com/microsoft/DirectXShaderCompiler.git`), `GLAD`
  (generated loader with local mesh-extension changes; generator is
  `https://github.com/Dav1dde/glad.git`), `mono` (headers/libs, source at
  `https://github.com/mono/mono.git`), `OpenSSL` (4.0.0 headers/import
  libraries/runtime DLLs), `PhysX` (5.4.0 headers/libs), `shaderc`
  (headers spanning shaderc/SPIRV-Tools components), `VulkanSDK` (a
  multi-component SDK package, not one repository), and `FBXSDK` (2020.3.7
  proprietary Autodesk binaries). If any of these move to source builds,
  treat that as a separate build and runtime compatibility milestone rather
  than part of the initial submodule migration.

## CMake build option and build-script layout (2026-10-19)

- [x] Document a direct Premake bootstrap for `GEngineTools.exe`. Scope: add
      the exact bundled-Premake invocation, `--qt6-sdk` generation condition,
      focused MSBuild command, and unified output path to the build guide.
      Acceptance: the command matches the current Premake project definition,
      states that `GEngineTools` itself does not link Qt, avoids any removed
      `.ps1` workflow reference, and `git diff --check` passes.
      Documentation-only; no engine rebuild is required.
      Implemented: the Premake build section now shows direct generation with
      the bundled executable, the focused MSBuild command, the unified
      `dist/premake-vs2022/.../GEngineTools.exe` output path, and the native
      tool's `qt-editor --premake` path. It also notes that the `--qt6-sdk`
      option currently gates target generation while `GEngineTools` itself does
      not link Qt.
      Verification (2026-10-27): reviewed the commands against
      `BuildScripts/Premake/GEngineTools.lua` and `premake5.lua`, confirmed no
      removed `.ps1` workflow appears in the section, and ran
      `git diff --check`. No engine rebuild was needed for this documentation-only
      change.

- [x] Add a fresh-environment quick start to the build guide and reference it
      from the root README. Scope: document recursive clone/LFS setup,
      `GEngineTools.exe` bootstrap, Mono SDK preparation, and complete CMake and
      Premake engine/example build paths with output locations. Acceptance:
      both generators use current commands, no removed `.ps1` workflow is
      referenced, the root README links to the guide without duplicating the full
      procedure, relative links resolve, and `git diff --check` passes. This is
      documentation-only; no engine rebuild is required.
      Implemented: `Documentation/Guides/BuildAndRun.md` now has a fresh
      environment quick start with recursive clone/LFS setup, CMake
      `GEngineTools.exe` bootstrap, Mono SDK preparation, CMake and Premake
      engine/example build commands, unified output paths, and execution from
      `Example/FrameGraphTriangle`. The root README links to that section
      without duplicating the full procedure. No engine API, architecture
      surface, or build configuration changed.
      Verification (2026-10-27): reviewed both generator command paths against
      the current launchers/generated project layout, checked the new Markdown
      links, found no removed `.ps1` workflow reference in the new quick start,
      and ran `git diff --check`. No engine rebuild was run for this
      documentation-only change.

- [x] Remove stale legacy third-party build outputs and generated project
      files. Scope: delete only the unreferenced legacy `box2d`/`ImGui`
      `.vcxproj` files, their filters, and legacy `bin`/`bin-int` directories
      under the third-party submodule; keep the active `GLAD` project files
      referenced by `GEngine.sln`, package runtime directories, and all
      unified `build`/`dist` outputs. Acceptance: stale paths are absent,
      the active solution still references the expected third-party projects,
      no tracked source or package payload changes, and `git diff --check`
      passes. This cleanup does not require an engine rebuild because it
      changes no build configuration or source.
      Implemented: removed the ten stale legacy paths under `box2d`, `ImGui`,
      and `GLAD`; retained the active `GLAD/GLAD.vcxproj` and filter files.
      Verification (2026-10-27): all stale paths are absent, the active GLAD
      project files remain present and referenced by `GEngine.sln`, the
      `box2d` and `ImGui` nested repository statuses are clean, and no tracked
      source or package payload changed. No engine rebuild was run because the
      build configuration and source were unchanged.

## Engine project split for compile throughput (planned)

- [ ] Split the current single `GEngine` project into focused build projects
      (for example, core/runtime, graphics, renderer, tools, each rendering
      backend, and optional editor-only services) so unrelated sources are not
      rebuilt together. Before implementation, inventory source coupling,
      public/private include boundaries, PCH ownership, generated code, backend
      dependencies, C# ScriptCore, and editor/application consumers. Preserve
      both Premake and CMake generation and keep application code backend-neutral.
      Acceptance: a baseline and post-split incremental build measurement shows
      which changed files rebuild only the owning project; full GEngine plus
      FrameGraphTriangle Debug/Release builds pass on Premake and CMake; bounded
      D3D12, OpenGL, and Vulkan runs pass; generated projects remain under the
      unified `build`/`dist` path contract; and public API/runtime behavior is
      unchanged. Record target boundaries, dependency graph, measured build times,
      and remaining coupling or deferred targets here.

- [x] Unify Premake and CMake output and intermediate paths. Rewrite the
  relevant build scripts/configuration so intermediates live under
  `build/<tool>/<system>-<architecture>/<configuration>/<target>/` and final
  artifacts live under
  `dist/<tool>/<system>-<architecture>/<configuration>/<target>/`. The tool
  component identifies Premake or CMake (including its Visual Studio action),
  while system, architecture, configuration, and target keep variants apart.
  Scope: inventory every Premake/CMake output and intermediate path, update
  target/debug/working directories and post-build copying, and keep CMake's
  current C# ScriptCore and Dist exceptions explicit. Acceptance: configure and
  build GEngine plus FrameGraphTriangle with each method in sequence; verify
  final executables, DLLs, import/static libraries, and PDBs appear in their
  `dist` target directories; verify objects/PCH/generated files remain under
  generator-isolated `build` directories; run a bounded FrameGraphTriangle
  readback; and document stale-artifact/concurrent-build guidance. Record
  actual verification and remaining target/configuration gaps here.
  Implemented: Premake's shared path component is now
  `premake-<action>/<system>-<architecture>/<configuration>`, every C++/C#
  project writes objects and PCH files under `build/` and final artifacts under
  `dist/`, and GEngine's DLL/runtime copy loop targets the new `dist` tree. The
  CMake launcher configures under `build/cmake-<action>/windows-x86_64/`; every
  CMake target uses matching `dist/cmake-<action>/windows-x86_64/<Config>/<Target>/`
  output properties, with compile PDBs kept under the CMake build tree. The
  C# ScriptCore output follows the unified Premake layout, and the editor
  post-build step copies it into its own `dist` target's `Resources/Scripts`
  directory. `.gitignore` now excludes `/dist/`.
  Verification (2026-10-20): Premake VS2022 generation passed and generated
  project files contain the new `OutDir`/`IntDir` paths. CMake VS2022
  configuration passed in an isolated VS developer child environment after the
  host's duplicate `PATH/Path` keys were normalized; generated project files
  contain the new `dist` output paths. Premake Debug/Release and CMake
  Debug/Release GEngine plus FrameGraphTriangle builds completed with existing
  warnings but zero errors. Premake Debug and CMake Release 120-frame D3D12
  runs each passed all three GPU readbacks and exited normally. C# ScriptCore
  Debug/Release builds passed and produced DLLs in their `dist` target paths.
  A GEngine-Editor Debug build attempt failed on pre-existing editor/API
  mismatches (`OnDetach` override and removed legacy renderer APIs), not on
  output paths; this does not block the engine/example path contract.
  Remaining limitations: VS2019/VS2026 and other application targets were not
  rebuilt; CMake still does not generate C# ScriptCore or Premake's `Dist`
  configuration. Legacy ignored `bin/` and `bin-int/` directories may remain
  locally but are no longer selected; delete them separately when stale-output
  cleanup is desired. The checked-in
  `GEngine-Editor/Resources/Scripts/GEngine-ScriptCore.dll` remains a legacy
  runtime payload rather than a new build output; removing it requires a
  separate editor build/deployment migration after the existing editor API
  mismatches are repaired.

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
