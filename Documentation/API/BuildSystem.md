# Build entry points

`GEngineTools.exe <mono-sdk|qt-sdk|qt-editor|run-qt-editor|export-project>`
is the native first-party workflow CLI. It is built from
`BuildScripts/Tools/GEngineTools.cpp`, links only Win32/BCrypt, and does not add
an engine public API. CMake and Premake both emit and deploy it under the
unified `dist` layout.

Console color preservation uses the existing GEngineTools Qt editor commands and Qt SDK;
no new flag is required. Full -Test also checks stream formatting and actual
engine info-log color. Regenerate/build the Host and EngineWorker together.

`GEngine-Editor/RunQtEditor.bat [Debug|Release] [Auto|CMake|Premake] [-PrintPath]`
defaults to Release/Auto. Auto picks the newest Host executable by UTC file
modification time across dist/qt-editor and dist/premake-vs2022 for that
configuration. Explicit generator selection never falls back. PrintPath prints
the selected absolute path without launching; missing builds/invalid arguments
return nonzero. Normal launch runs from the Host directory and returns its exit code.

Editor E1.1 project/assets browsing uses the same GEngineTools Qt editor commands and Qt
SDK as E0.2. No new build flags or deployment dependencies are required; CMake and
Premake both include ProjectWorkspace, and full -Test covers its project lifecycle.
E1.4 SelectionService is a header-only Core type. ProtocolTests include it directly,
while the Qt Host consumes it through ProjectWorkspace; no generator source list,
Qt module or engine dependency changes.

`GEngineTools.exe mono-sdk` installs the official Mono 6.10.0.87 x64 SDK
under `dist/mono/windows-x86_64/6.10.0.87/Mono`. CMake and Premake reference
that SDK’s `include/mono-2.0` directory and static library. The command does
not add a
public engine API, alter runtime BCL resolution, or implement export/packaging.
Debug editor/engine and Release engine builds have passed.

Shared Mono development paths and incremental export are documented as
[planned contracts](ManagedScripting.md). No new build arguments or packaging
commands exist yet; current commands and runtime copying remain unchanged.

Qt source location: `GEngine/GEngine-ThirdParty/qtbase` (v6.10.3,
`7ddbc87d8e14ce51d2957ea72d0a6077593d5ff4`). Initialize with
`git submodule update --init --recursive` after the aggregate gitlink is published.
This provides editor-only Core/Gui/Widgets source, not Qt6Config.cmake from an
installed SDK. Current engine/game targets do not compile, link or deploy Qt;
their public APIs and generator commands remain unchanged.

The E0.1 Qt Host now uses find_package(Qt6 6.10.3 EXACT COMPONENTS Core Gui
Widgets) and matching imported targets. `GEngineTools.exe qt-sdk` and
`GEngineTools.exe qt-editor` provide opt-in SDK/build/test commands; the latter
supports `--premake` or `--protocol-only`. See the editor README for arguments and
outputs. Default runtime/game generation still works without Qt. The root CMake
project provides GEngine-EditorEngineWorker without requiring Qt. Full editor
builds deploy that real render child; -ProtocolOnly skips it. Premake's optional
Qt target set includes the GEngine-linked worker and its runtime dependencies.

The legacy GEngine-Editor target, launcher and ImGuizmo source/include entries
have been removed from both generators. The unused ImGuizmo submodule remains
in the aggregate repository; Qt Host/Worker and engine/game ImGui are unaffected.

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

No engine application-facing C++ API changes were made. CMake generates the C#
`GEngine-ScriptCore` target when `GE_WITH_MONO` is ON, and omits it for
native-only builds with `GE_WITH_MONO=OFF`. Premake's `Dist` configuration is
not exposed by this CMake launcher.
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

## Native project builds

The automatic project pipeline uses BuildScripts/Project/CMakeLists.txt, discovers
src/*.cpp recursively, builds Release with GE_PACKAGED_GAME=1 and GE_WITH_MONO=OFF,
and links/deploys the current GEngine and native runtime dependencies. It never
asks for a prebuilt game EXE. `GE_PROJECT_NAME` controls both launcher and game
output names; `GE_PROJECT_ICON` optionally configures an embedded launcher icon.
`GEngineTools export-project --executable-name <base-name> --icon <file.ico>`
provides those values. Omitting `--icon` clears a previous cached icon selection;
unchanged icon bytes reuse a stable cached resource file for incremental builds.
`GProjectPackage` always runs CopyRuntime after its game/launcher dependencies,
so an engine-only DLL change reaches the package even without game relinking.
The generated package root contains only the launcher EXE plus `Runtime/`, `Data/`,
and `Metadata/`; the real game EXE and DLLs are under `Runtime/`. General CMake
GE_WITH_MONO defaults ON; OFF omits managed SDK linking and explicitly rejects
script initialization. Premake supports the equivalent --without-mono opt-in.
GE_DIST_ROOT is a cache PATH override for isolated output roots; its default retains
the standard build/dist convention.
