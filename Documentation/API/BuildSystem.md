# Build entry points

## GAsset conversion

`GEngineTools.exe import-resource --source <file> --output <new.gasset>` imports
one supported source file (at most 64 MiB) without deleting it or rewriting project
references. The destination directory must exist. Existing targets and linked paths
are rejected. Output is staged in that directory, checked, then renamed without
replacement; errors return nonzero and remove staging files. This is the editor's
native import endpoint. It wraps source bytes without validating image/FBX contents
or collecting dependencies.

`GEngineTools.exe gasset [--root <repository>]`
converts supported resources in place to `.gasset` and scenes to `.gscene`,
updating project references. Back up the selected checkout first. The root
defaults to the engine checkout. Add `--repair-references` to repair references
to existing assets without conversion; `--help` prints usage without changes.
Conversion and repair are implemented directly in C++; no Python interpreter or
conversion script is required. Errors return nonzero. The tool scans Example/Sandbox Assets/Resources
and editor resource roots rather than arbitrary standalone files. Runtime GAsset
APIs and backend ownership are unchanged. Text payloads use YAML double-quoted
scalars preserving whitespace; binary payloads use YAML Base64. Payloads and
destination collisions are checked before writes. The batch is not atomic:
I/O failure may leave partial output; keep a backup.

## Workflow commands

`GEngineTools.exe <gasset|import-resource|mono-sdk|qt-sdk|qt-editor|run-qt-editor|export-project>`
is the native first-party workflow CLI. It is built from
`BuildScripts/Tools/GEngineTools.cpp`, links only Win32/BCrypt, and does not add
an engine public API. CMake and Premake both emit and deploy it under the
unified `dist` layout.

Editor E1.7 adds optional settings.defaultScene to schema-1 project files.
export-project accepts this editor metadata without changing native game startup
or packaging options. The tool checks the settings object/known string field;
the Host owns scene existence and project-root validation. CMake explicitly lists
ProjectSettingsCheck.cpp and its CTest; Premake's existing Host glob covers it.
No additional Qt module, vendor library or engine public API is introduced.

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
public engine build API changes. Both CMake and Premake require an installed
Vulkan SDK with the optional VMA component whose root is exported as
`VULKAN_SDK`; generation fails with an actionable error when it is missing or
incomplete. The submodule is pinned at
`99c4b5e529f402e2292d0e367fabe5c356bbe995`; application project files and
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
Both generators expose the engine backend source sets as the static targets
`GEngine-OpenGLBackend`, `GEngine-VulkanBackend`, and `GEngine-D3D12Backend`;
`GEngine` links all three by default. No build flag, public C++ signature, or
runtime backend-selection API is added by this split.
The physics service split adds the static targets
`GEngine-Physics2DBackend` and `GEngine-Physics3DBackend`. `GEngine` links both
modules, which own the Box2D/PhysX dependencies. These are native service
backends, not graphics APIs.
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
`export-project` publishes Config/Config.ini separately from the asset pack,
generating defaults if the project has no config. Existing destination config and
Cache/, Logs/, UserData/ are retained when updating a managed package; native
binaries/archive still use strict modification checks. Empty asset packs are valid.
The generated package root contains only the launcher EXE plus `Runtime/`, `Data/`,
and `Metadata/`; the real game EXE and DLLs are under `Runtime/`. General CMake
GE_WITH_MONO defaults ON; OFF omits managed SDK linking and explicitly rejects
script initialization. Premake supports the equivalent --without-mono opt-in.
GE_DIST_ROOT is a cache PATH override for isolated output roots; its default retains
the standard build/dist convention.
