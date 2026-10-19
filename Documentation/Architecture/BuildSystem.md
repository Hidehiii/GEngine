# Build system architecture

Premake remains the established Visual Studio generator. The separate CMake
generator lives under `BuildScripts/CMake/`, uses the same first-party engine
and vendored source tree, and writes to `build/cmake-<version>/` so generated
projects and objects do not contend with Premake outputs. Two launchers under
`BuildScripts/` select Premake or CMake. The bundled Premake executable is in
`BuildScripts/Premake/premake5.exe`; engine dependencies remain under `GEngine/vendor/`.
The CMake graph has five vendored C++ libraries, the shared GEngine library,
seven C++ examples, Sandbox, and Editor. It copies the engine and required
vendor runtime DLLs beside each built executable. Premake alone currently
generates the C# ScriptCore project.

Planned, not implemented: migrate the engine dependency tree from
`GEngine/vendor/` to an aggregate `GEngine/Third-Party/` submodule. Its remote
repository must be identified or created before conversion; local patches,
binary SDK packages, and the nested `SPIRV-Reflect` gitlink must be preserved.
Until that migration is verified, all build paths above still refer to
`GEngine/vendor/`.

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
The Release configuration was generated and built with `VULKAN_SDK` unset,
using checked-in Vulkan headers and libraries. Other CMake application
targets were generated but not individually built. The CMake option does not
currently generate Premake's `Dist` or C# ScriptCore project.

This build-only change does not alter runtime ownership or backend behavior.
See the [build guide](../Guides/BuildAndRun.md) for supported generators,
targets, toolchain requirements, and parity limits.
