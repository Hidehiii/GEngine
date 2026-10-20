# Build system architecture

Premake remains the established Visual Studio generator. The separate CMake
generator lives under `BuildScripts/CMake/`, uses the same first-party engine
and vendored source tree. Two launchers under `BuildScripts/` select Premake
or CMake. The bundled Premake executable is in
`BuildScripts/Premake/premake5.exe`; engine dependencies remain under
`GEngine/vendor/`. The CMake graph has five vendored C++ libraries, the shared
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
`GEngine/vendor/`. The migration proposal for an aggregate
`GEngine/Third-Party/` submodule, including its provenance and preservation
requirements, is owned by [Workflow.md](../../TODO/Workflow.md) and is not
implemented.

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
