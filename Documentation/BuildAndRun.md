# Build and run

## Prerequisites

Development is currently Windows-first. Install the following before generating the solution:

- Visual Studio 2022 with Desktop development with C++ and the Windows SDK.
- Vulkan SDK, including the headers, loader library and shader tools used by the project.
- .NET Framework 4.7.2 Developer Pack, required by the current solution tooling.

The repository vendors most runtime libraries under `GEngine/vendor`; do not replace those folders with global installations unless you also update `Dependencies.lua`.

## Generate the solution

From the repository root, run:

```bat
Scripts\GenerateProject.bat
```

This regenerates `GEngine.sln` and Visual Studio project files from the Premake scripts. Changes intended to survive regeneration belong in `premake5.lua` files, not only in `.vcxproj` files.

## Build order

1. Open `GEngine.sln`.
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

`GraphicsAPI: 3` selects D3D12 in the current configuration. Keep shader assets and the cache directory relative to the executable's working directory, not the solution root.

## Adding an example

1. Create a directory under `Example/<Name>` with a `premake5.lua`, `Config.ini`, `Assets` and `src` directory.
2. Add the project include in `Example/premake5.lua`.
3. Implement `CreateApplication` and one or more `Layer` classes.
4. Regenerate the solution, then build the new project.

Use `Example/Triangle` as the smallest reference application. It deliberately has no editor, ImGui or scene dependency.
