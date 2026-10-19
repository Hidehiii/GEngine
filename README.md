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

### Example project
You could find some example projects in the Example folder, and you could run them after building the engine core and example project.

### Documentation
#### Shader(.shader)

[Example shader](Example/ShaderCompile/Assets/Shaders/ExampleShader.shader)
