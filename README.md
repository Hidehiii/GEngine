# GEngine

A game engine.

Its rendering is developed based on the OpenGL, Vulkan and Dx12, the 2D physics is based on the Box2D library, and the 3D physics is based on Nvidia's PhysX library.

### Dependencies
##### [VulkanSDK](https://www.lunarg.com/vulkan-sdk/)
Version: windows x64 

All components should be installed.
##### [.Net Framework](https://dotnet.microsoft.com/zh-cn/download/visual-studio-sdks?cid=getdotnetsdk)
Version: .Net Framewor 4.7.2 Developer Pack

### Start
Run [Scripts/GenerateProjects.bat](Scripts/GenerateProjects.bat) and select VS2022, VS2019, or VS2026. The bundled Premake 5.0.0-beta8 supports all three options; see [Build and run](Documentation/BuildAndRun.md). Open the generated solution with Visual Studio and build the desired example.

### Example project
You could find some example projects in the Example folder, and you could run them after building the engine core and example project.

### Documentation
#### Shader(.shader)

[Example shader](Example/ShaderCompile/Assets/Shaders/ExampleShader.shader)
