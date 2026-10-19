# Build entry points

`BuildScripts/GenerateProjectsPremake.bat [vs2022|vs2019|vs2026]` retains the
Visual Studio selection and `PREMAKE_EXE` override. The sibling
`BuildScripts/GenerateProjectsCMake.bat [vs2022|vs2019]` configures the CMake
project from `BuildScripts/CMake/` into `build/cmake-<version>/`, defaulting to
VS2022. Set `CMAKE_EXE` to override the CMake executable. Both launchers accept
calls from any working directory and return a nonzero status on failure.
The launcher's bundled fallback is `BuildScripts/Premake/premake5.exe`;
`PREMAKE_EXE` remains the optional executable override.
The fallback path was verified by generating VS2022 projects from a different
working directory.

Planned, not implemented: the Premake and CMake generators will resolve engine
dependencies from `GEngine/Third-Party/` after a submodule migration. Their
current commands and paths remain unchanged. No public engine C++ API change
is expected; the contributor-facing clone/setup instructions will need to
include recursive submodule initialization once verified.
For example, build the generated CMake target with
`cmake --build build/cmake-vs2022 --config Debug --target FrameGraphTriangle`.

The CMake targets define `UNICODE` and `_UNICODE` for existing Win32 callsites.
No public C++ signatures change with this build configuration.
The CMake build uses checked-in OpenSSL import libraries at `VC/x64/MD[d]`.
An empty vendor runtime-DLL match causes an explicit post-build error.

No engine application-facing C++ API changes were made. C# ScriptCore project
generation remains Premake-only; see the [build guide](../Guides/BuildAndRun.md).
Premake's `Dist` configuration is also not exposed by this CMake launcher.
VS2022 generation was checked from outside the repository directory for both
launchers; CMake Debug/Release engine/example builds and 120-frame example
readbacks were verified. VS2019 and other CMake application builds remain
unverified.

Planned, not implemented: the CMake build will eventually put engine and
application binaries in the same `bin/<Configuration>-windows-x86_64/<Target>/`
locations as Premake and align other target artifacts using an explicit path
map. Current CMake paths above remain accurate until that work is
implemented. This is a contributor-facing output contract, not a change to
public engine C++ signatures. Do not run both generators concurrently against
one shared final output directory.
