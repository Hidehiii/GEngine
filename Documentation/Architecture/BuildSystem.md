# Build system architecture

Premake remains the established Visual Studio generator. The separate CMake
generator lives under `BuildScripts/CMake/`, uses the same first-party engine
and third-party source tree. Two launchers under `BuildScripts/` select Premake
or CMake. The bundled Premake executable is in
`BuildScripts/Premake/premake5.exe`; engine dependencies are pinned in the
`GEngine/Third-Party` aggregate submodule. The CMake graph has five vendored C++ libraries, the shared
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
`GEngine/Third-Party/`, which points at
`https://github.com/Hidehiii/GEngine-ThirdParty.git` and preserves
`SPIRV-Reflect` as a nested pinned submodule. The aggregate repository's
[INVENTORY.md](../../GEngine/Third-Party/INVENTORY.md) records provenance,
local overlays, binary assets, and license gaps. Fresh recursive-clone
verification is recorded with the migration work item in
[ThirdPartyDependencies.md](../../TODO/Build/ThirdPartyDependencies.md).
The main repository pins aggregate commit
`8942f70419de3981ad26ac3b747860e00da2b9be`; its snapshot contains the
originally tracked vendor paths plus aggregate metadata, excludes generated
local build outputs, and uses aggregate-local submodules for
`DirectX-Headers`, `json`, and `SPIRV-Reflect`. It passed a fresh recursive
Git LFS clone.

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
