# GEngine

A game engine.

Its rendering is developed based on the OpenGL, Vulkan and Dx12, the 2D physics is based on the Box2D library, and the 3D physics is based on Nvidia's PhysX library.

### Dependencies
##### [VulkanSDK](https://www.lunarg.com/vulkan-sdk/)
The repository includes Windows x64 Vulkan headers, libraries, and shader
tools. Set `VULKAN_SDK` only to override the bundled SDK.
##### [.Net Framework](https://dotnet.microsoft.com/zh-cn/download/visual-studio-sdks?cid=getdotnetsdk)
Version: .NET Framework 4.7.2 Developer Pack (for the C# ScriptCore project).

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
