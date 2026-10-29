# GEngine

A game engine.

Its rendering is developed based on the OpenGL, Vulkan and Dx12, the 2D physics is based on the Box2D library, and the 3D physics is based on Nvidia's PhysX library.

### Required dependencies

- A Windows 10 or newer development environment.
- Visual Studio 2026, 2022, or 2019 with Desktop development with C++ and the
  matching Windows SDK installed.
- Git and Git LFS installed, with the repository cloned recursively so
  `GEngine/GEngine-ThirdParty` is present.
- [.NET Framework 4.7.2 Developer Pack](https://dotnet.microsoft.com/en-us/download/dotnet-framework/net472)
  installed.
- The official Mono 6.10.0.87 SDK prepared under
  `dist/mono/windows-x86_64/6.10.0.87/Mono`; run
  `GEngineTools.exe mono-sdk` once after the workflow tool is built.
- The [Vulkan SDK](https://vulkan.lunarg.com/sdk/latest/windows)
  installed and its root exported as `VULKAN_SDK`; both CMake and Premake
  require this environment variable.

Additional requirements depend on the selected build path:

- CMake builds require CMake 3.24 or newer.
- Building the Qt editor requires Python and a locally built Qt 6.10.3 SDK;
  `GEngineTools.exe qt-sdk` builds that SDK.
- Vulkan headers, libraries, shader tools, and runtime DLLs come from the
  installed SDK selected by `VULKAN_SDK`.

### Start
Generate projects with [Premake](BuildScripts/GenerateProjectsPremake.bat) (VS2022, VS2019, or VS2026) or [CMake](BuildScripts/GenerateProjectsCMake.bat) (VS2022 or VS2019). See [Build and run](Documentation/Guides/BuildAndRun.md) for commands, target coverage, and output paths.
For a new machine, start with the [fresh environment quick start](Documentation/Guides/BuildAndRun.md#fresh-environment-quick-start).

### Example project
You could find some example projects in the Example folder, and you could run them after building the engine core and example project.

### Documentation

Start with the [documentation home](Documentation/README.md). It indexes the
project's architecture, API, guides, workflows, diagnostics, examples, editor,
tests, and engineering standards.

### TODO

Track implementation milestones and verification evidence in the
[workflow TODO index](TODO/Workflow.md).
