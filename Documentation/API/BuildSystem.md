# Build entry points

Initialize nested `ImGuizmo` with the other submodules. Both generators resolve
headers from `ImGuizmo/src` and compile its main source only into GEngine-Editor.
The old engine compatibility source is removed; no upstream engine export patch
is required. Generator commands are unchanged.

The in-progress ImGui/Box2D/SPIRV-Cross upgrade retains existing generator
commands. Regenerate projects: ImGui lives at `imgui`, SPIRV-Cross at
`SPIRV-Cross`, and core/GLSL are source-built targets. Rendering-device APIs are
unchanged. See [upgrade tracking](../../TODO/Build/ThirdPartyDependencies.md).

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

The generators resolve engine dependencies from the `GEngine/GEngine-ThirdParty/`
aggregate submodule. Recursive submodule initialization and Git LFS are build
prerequisites; the migration status and fresh-clone evidence are owned by
[ThirdPartyDependencies.md](../../TODO/Build/ThirdPartyDependencies.md). No
public engine build API changes. The submodule is pinned at
`91ffb9e797f2e0d8e34571cef4dda14cbcc6e29a`; application project files and
runtime DLL copy commands consume the same path without new public C++
signatures. `DirectX-Headers`, `jsoncpp`, `HLSLcc`, `entt`, `glm`, `glfw`,
`yaml-cpp`, `spdlog`, and `stb` are aggregate-local
submodules.
Submodule directory names match their remote repository names; for example, the
engine-level aggregate is `GEngine/GEngine-ThirdParty` and JsonCpp is checked out
under `jsoncpp`.
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
On 2026-10-21, Premake and CMake Debug/Release GEngine and FrameGraphTriangle
builds and 120-frame D3D12/OpenGL/Vulkan runs passed in both configurations
after the Third-Party submodule migration.
The same verification passed after replacing the five newest source
dependencies with aggregate-local upstream submodules, and a fresh recursive
Git LFS clone reproduced the pinned tree.

CMake and Premake both write final artifacts to the `dist` contract above;
their generator-specific roots prevent object/PCH contention and stale
cross-tool artifacts.
