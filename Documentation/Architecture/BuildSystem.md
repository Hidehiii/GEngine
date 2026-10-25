# Build system architecture

Editor ConsoleOutput is a Qt-only Host source, explicitly listed by the standalone
CMake build and discovered by Premake's existing Host glob. The worker reuses its
existing spdlog dependency for ANSI stderr; no new module or game dependency.

GEngine-Editor/RunQtEditor.bat delegates to its sibling PowerShell launcher. Auto
selects the newest Host executable in the requested configuration across the
standalone CMake and Premake outputs, then runs from that executable's directory.
Explicit generator selection and PrintPath support diagnosing stale outputs.
No engine build interface or Qt deployment dependency changes.

Editor E1.1 adds Core/ProjectContext and Host/ProjectWorkspace. CMake explicitly
lists the new Host implementation; Premake's existing Host/Core discovery picks
it up. The read-only filesystem model uses existing Qt modules, with no engine
or vendor dependency changes.

Planned C# deployment separates shared development Runtime SDKs from incremental
game export; see [managed scripting design](ManagedScripting.md). No build/copy
behavior changes in this documentation work item. CMake C# support remains pending.

Qt 6 source is now stored as the editor-only `qtbase` nested submodule under
`GEngine/GEngine-ThirdParty`, pinned to tag v6.10.3 / commit
`7ddbc87d8e14ce51d2957ea72d0a6077593d5ff4`. Core/Gui/Widgets sources are
present; neither build generator links Qt into GEngine or games. The original
source-only addition is now followed by an opt-in Qt SDK/Host prototype build.

Planned UI split: a separate Qt 6 Widgets EditorHost links Qt Core/Gui/Widgets;
EngineWorker/game link GEngine and Dear ImGui, without Qt. The legacy ImGui editor target and launcher have been removed; Qt Host is the only editor. BuildQt6.ps1 installs
matching Debug/Release SDKs out of source. BuildQtEditor.ps1 builds independent
Host, DiagnosticWorker, EngineWorker and ProtocolTests targets using CMake or optional Premake.
Only EngineWorker links GEngine; Host remains Qt-only. The standalone build script
also builds EngineWorker through the root MSVC CMake project and deploys it beside
the Host's sibling target directories; protocol-only builds skip the engine.
SDK/target outputs are isolated under build/qt6, dist/qt6 and build/dist/qt-editor;
Premake uses the standard premake-vs2022 paths. The root CMake option
GE_BUILD_QT_EDITOR is OFF by default; Premake opts in with --qt6-sdk. Full engine
integration remains in [editor tracking](../../TODO/EditorArchitecture.md).
When opted into the root CMake project, editor outputs use that project's tool
identifier; the standalone Ninja build uses qt-editor. SDK prefixes must match
the requested configuration. Editor classes use lambda/callback connections,
so the E0.1 application does not yet require custom QObject moc generation.

ImGuizmo is a nested upstream submodule pinned to
`105e4fde45e4e74aae8973d98750820ef7aca7e2` (header v1.92.5 WIP).
It has no build consumers after legacy editor removal; the source pin remains
in the shared aggregate repository. Engine/game ImGui is retained.

Upgrade in progress: upstream `imgui`, `box2d`, and `SPIRV-Cross` submodules
replace snapshots. Premake overlays live in `BuildScripts/Premake`. Both build
systems compile SPIRV-Cross core/GLSL from the same tree as its headers, replacing
SDK prebuilt libraries. Verification and aggregate publication remain tracked in
the [dependency TODO](../../TODO/Build/ThirdPartyDependencies.md).

Premake remains the established Visual Studio generator. The separate CMake
generator lives under `BuildScripts/CMake/`, uses the same first-party engine
and third-party source tree. Two launchers under `BuildScripts/` select Premake
or CMake. The bundled Premake executable is in
`BuildScripts/Premake/premake5.exe`; engine dependencies are pinned in the
`GEngine/GEngine-ThirdParty` aggregate submodule. The CMake graph has five vendored C++ libraries, the shared
GEngine library, seven C++ examples, Sandbox, and Editor. It copies the engine
and required vendor runtime DLLs beside each built executable. Premake alone
currently generates the C# ScriptCore project.

Both generators use one path contract keyed by generator, system,
architecture, configuration, and target. Intermediates live under
`build/<tool>/<system>-<architecture>/<configuration>/<target>/`; final
executables, DLLs, import/static libraries, linker PDBs, and copied runtime
DLLs live under
`dist/<tool>/<system>-<architecture>/<configuration>/<target>/`. The tool
component is, for example, `premake-vs2022` or `cmake-vs2022`. Premake objects
and PCH files use the target's `build` directory. CMake's generated projects,
cache, objects, and PCH files use
`build/cmake-<action>/<system>-<architecture>/`; its linker outputs and
runtime payloads use the matching `dist` tree. The C# ScriptCore assembly
follows the same Premake `build`/`dist` layout, and the editor post-build step
copies it into the editor target's `Resources/Scripts` directory. Legacy
`bin/` and `bin-int/` directories are no longer selected by either generator.

All current build paths resolve the engine dependency tree from
`GEngine/GEngine-ThirdParty/`, which points at
`https://github.com/Hidehiii/GEngine-ThirdParty.git` and preserves
`SPIRV-Reflect` as a nested pinned submodule. The aggregate repository's
[INVENTORY.md](../../GEngine/GEngine-ThirdParty/INVENTORY.md) records provenance,
local overlays, binary assets, and license gaps. Fresh recursive-clone
verification is recorded with the migration work item in
[ThirdPartyDependencies.md](../../TODO/Build/ThirdPartyDependencies.md).
The main repository pins aggregate commit
`91ffb9e797f2e0d8e34571cef4dda14cbcc6e29a`; its snapshot contains the
originally tracked vendor paths plus aggregate metadata, excludes generated
local build outputs, and uses aggregate-local submodules for
`DirectX-Headers`, `jsoncpp`, `HLSLcc`, `entt`, `glm`, `glfw`, `yaml-cpp`,
`spdlog`, `stb`, and `SPIRV-Reflect`. It passed a
fresh recursive
Git LFS clone.
Submodule directory names match their remote repository names. The engine-level
aggregate directory is `GEngine/GEngine-ThirdParty`, and aggregate-local
submodule directories such as `jsoncpp` use the same rule; new submodules must
follow it when they are introduced.

The bundled executable sits directly in `BuildScripts/Premake/`; the previous
`bin/premake` nesting was removed. This only changes the launcher's lookup path.
Its file hash was unchanged, and VS2022 generation passed from outside the
repository root; no engine rebuild was needed for this layout-only edit.

The first CMake Debug compile found wide-string Win32 calls resolving to ANSI
entry points. The CMake targets now define `UNICODE` and `_UNICODE` explicitly;
this corrects build configuration without changing runtime APIs.
The CMake link graph resolves OpenSSL import libraries from the checked-in
`VC/x64/MD[d]` layout and uses the vendored Vulkan SDK unless overridden.
The post-build copy script requires vendor DLL matches so a missing runtime
bundle fails the build instead of producing an executable that cannot start.

VS2022 CMake Debug/Release builds of GEngine and FrameGraphTriangle passed.
Premake Debug/Release and CMake Debug/Release builds of GEngine and
FrameGraphTriangle also passed on 2026-10-20 using the unified layout.
Premake Debug and CMake Release 120-frame D3D12 runs passed all three GPU
readbacks and exited normally. C# ScriptCore Debug/Release builds passed.
On 2026-10-21, the same engine/example targets rebuilt successfully with both
Premake and CMake after the Third-Party submodule migration. CMake 120-frame
D3D12, OpenGL, and Vulkan runs passed in both Debug and Release, and Premake
D3D12 runs also passed in both configurations.
The same build and backend checks passed after replacing `glm`, `glfw`,
`yaml-cpp`, `spdlog`, and `stb` with aggregate-local upstream submodules, and a
fresh recursive Git LFS clone reproduced the pinned tree.
Other application targets, VS2019, and VS2026 were not rebuilt for this path
change. The CMake option does not generate Premake's `Dist` or C# ScriptCore
project. A GEngine-Editor Debug build attempt still fails on pre-existing
editor/API mismatches such as an invalid `OnDetach` override and removed
legacy renderer calls; that failure is independent of the new path layout.

Generator-specific `build` and `dist` roots keep objects, PCH files, and final
artifacts isolated even when two tools target the same system, architecture,
configuration, and target. Do not start two builds for the same tool and
configuration while either writes those directories.

This build-only change does not alter runtime ownership or backend behavior.
See the [build guide](../Guides/BuildAndRun.md) for supported generators,
targets, toolchain requirements, and parity limits.
