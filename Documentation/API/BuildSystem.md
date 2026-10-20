# Build entry points

`BuildScripts/GenerateProjectsPremake.bat [vs2022|vs2019|vs2026]` retains the
Visual Studio selection and `PREMAKE_EXE` override. The sibling
`BuildScripts/GenerateProjectsCMake.bat [vs2022|vs2019]` configures the CMake
project from `BuildScripts/CMake/` into
`build/cmake-<action>/windows-x86_64/`, defaulting to VS2022. Set `CMAKE_EXE`
to override the CMake executable. Both launchers accept calls from any working
directory and return a nonzero status on failure.
The launcher's bundled fallback is `BuildScripts/Premake/premake5.exe`;
`PREMAKE_EXE` remains the optional executable override.
The fallback path was verified by generating VS2022 projects from a different
working directory.

The generators currently resolve engine dependencies from `GEngine/vendor/`.
The proposed `GEngine/Third-Party/` submodule migration and recursive-clone
setup requirements are owned by
[Workflow.md](../../TODO/Workflow.md), not implemented by this page. Its
2026-10-20 dependency inventory identifies candidate upstream repositories,
but no public engine build API or current path contract changes.
For example, build the generated CMake target with
`cmake --build build/cmake-vs2022/windows-x86_64 --config Debug --target FrameGraphTriangle`.

All generated targets use
`build/<tool>/windows-x86_64/<Configuration>/<Target>/` for objects and PCH
files and `dist/<tool>/windows-x86_64/<Configuration>/<Target>/` for final
binaries, libraries, PDBs, and copied runtime DLLs. `<tool>` is
`premake-vs2022`, `premake-vs2019`, `premake-vs2026`, or `cmake-vs2022`.
For example, the VS2022 Premake Debug executable is
`dist/premake-vs2022/windows-x86_64/Debug/FrameGraphTriangle/FrameGraphTriangle.exe`.

The CMake targets define `UNICODE` and `_UNICODE` for existing Win32 callsites.
No public C++ signatures change with this build configuration.
The CMake build uses checked-in OpenSSL import libraries at `VC/x64/MD[d]`.
An empty vendor runtime-DLL match causes an explicit post-build error.

No engine application-facing C++ API changes were made. C# ScriptCore project
generation remains Premake-only; see the [build guide](../Guides/BuildAndRun.md).
Premake's `Dist` configuration is also not exposed by this CMake launcher.
VS2022 generation was checked from outside the repository directory for both
launchers. On 2026-10-20, Premake and CMake Debug/Release GEngine plus
FrameGraphTriangle builds passed in the unified layout, and 120-frame D3D12
runs from Premake Debug and CMake Release passed all three GPU readbacks.
C# ScriptCore Debug/Release output paths were also verified. VS2019, VS2026,
and other application targets remain unverified for this path change.

CMake and Premake both write final artifacts to the `dist` contract above;
their generator-specific roots prevent object/PCH contention and stale
cross-tool artifacts.
