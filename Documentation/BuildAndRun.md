# Build and run

## Prerequisites

Development is currently Windows-first. Install the following before generating the solution:

- Visual Studio 2026, 2022 or 2019 with Desktop development with C++ and the Windows SDK.
- Premake 5.0.0-beta8 is bundled under `vender/bin/premake` and supports
  VS2026, VS2022 and VS2019 project generation.
- The repository includes the Vulkan headers, loader library and shader tools
  under `GEngine/vendor/VulkanSDK`. Set `VULKAN_SDK` only to use another
  installed SDK.
- .NET Framework 4.7.2 Developer Pack, required by the current solution tooling.

The repository vendors most runtime libraries under `GEngine/vendor`; do not replace those folders with global installations unless you also update `Dependencies.lua`.

## Generate the solution

From the repository root, run:

```bat
Scripts\GenerateProjects.bat
```

Choose `1` for VS2022, `2` for VS2019, or `3` for VS2026. The script also
accepts `vs2022`, `vs2019`, or `vs2026` as an argument for noninteractive use,
and can be launched from any working directory.

To generate for VS2026 using the bundled executable:

```bat
Scripts\GenerateProjects.bat vs2026
```

Optionally, set `PREMAKE_EXE` to the full path of another compatible executable.
If an older override is already set, clear it with `set "PREMAKE_EXE="` in
Command Prompt, or `Remove-Item Env:PREMAKE_EXE -ErrorAction SilentlyContinue`
in PowerShell, to use the bundled version.

Without `PREMAKE_EXE`, the script uses the bundled executable. Selecting VS2026
with an unsupported executable fails explicitly; it does not fall back to
VS2022. Missing executables and generation failures return a nonzero exit code.
The menu pauses before returning; command-line invocation does not pause.

Premake beta8 reports deprecation warnings for existing `flags` settings
(`NoRuntimeChecks`, `NoIncrementalLink`, and `NoPCH`). These warnings are not
generation failures; migrating those settings is outside this binary upgrade.

This regenerates the Visual Studio solution and project files from the Premake scripts. VS2026 emits `GEngine.slnx`; VS2022 and VS2019 emit `GEngine.sln`. Changes intended to survive regeneration belong in `premake5.lua` files, not only in `.vcxproj` files.

Workspace Release and Dist configurations define `NDEBUG`; Debug defines
`_DEBUG`. PhysX requires exactly one of these definitions. Regenerate existing
projects after pulling this configuration change rather than editing generated
project definitions individually.

For a bounded FrameGraphTriangle run, set `GENGINE_EXAMPLE_FRAME_LIMIT=120`
or `GENGINE_EXAMPLE_SECONDS=20` in its launch environment. Leave the other limit
unset (or zero); if both are positive, the first reached requests normal
application shutdown. Timed runs use a monotonic wall clock. Launch from
`Example/FrameGraphTriangle` so configuration and shader paths resolve.

## Build order

1. Open `GEngine.slnx` for VS2026, or `GEngine.sln` for VS2022/VS2019.
2. Select `Debug | x64` for development, or `Release | x64` for optimized testing.
3. Build the desired example project, such as `Triangle`. Its project dependency builds `GEngine` first and copies the current DLL to the example output directory.
4. Set the example as Startup Project and run it.

The Triangle executable is emitted under `bin/Debug-windows-x86_64/Triangle` for the Debug x64 configuration.

## Parallel build guidance

Use one build invocation per configuration/output directory. Starting a Visual Studio build and a command-line build at the same time can contend for `GEngine.pch`.

The Debug GEngine project uses `/FS` to serialize concurrent writes to its compiler PDB (`vc143.pdb`) when MSVC compiles source files in parallel. This addresses the internal PDB contention, but it cannot make two separate builds safely share one intermediate directory.

If a previous build was interrupted and the compiler reports that `GEngine.pch` is in use:

1. Wait for any `cl.exe`/MSBuild process for that build to finish.
2. Ensure only one Visual Studio or command-line build remains.
3. Rebuild the affected project. Use Clean only when the build output is no longer in use.

## Configuration

Each executable loads its local `Config.ini` from its working directory. Important values include:

```ini
GraphicsAPI: 3
FramesInFlight: 2
VSync: 0
CommandBufferCount: 8
ShaderCacheDirectory: Assets/Cache/Shaders/
WindowManagerAPI: 0
```

For an independent-queue Vulkan diagnostic run, set
`GENGINE_VULKAN_DEDICATED_QUEUES=1`. When supported, the backend selects
non-graphics compute/transfer families and logs their indices. Defaults retain
the existing shared-family policy. This is separate from `GraphicsAPI: 2`.

`GraphicsAPI: 3` selects D3D12 in the current configuration. Keep shader assets and the cache directory relative to the executable's working directory, not the solution root.

## Adding an example

1. Create a directory under `Example/<Name>` with a `premake5.lua`, `Config.ini`, `Assets` and `src` directory.
2. Add the project include in `Example/premake5.lua`.
3. Implement `CreateApplication` and one or more `Layer` classes.
4. Regenerate the solution, then build the new project.

Use `Example/Triangle` as the smallest reference application. It deliberately has no editor, ImGui or scene dependency.
