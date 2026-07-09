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
run [Scripts/GenerateProject.bat](Scripts/GenerateProject.bat) and everything of project would be setted up. Just open the project with Visual Studio and build all. After building Example project, you need to build the engine core again so that the engine core .dll file would auto copy to Example project.

### Example project
You could find some example projects in the Example folder, and you could run them after building the engine core and example project.

### Documentation
#### Shader(.shader)

[Example shader](Example/ShaderCompile/Assets/Shaders/ExampleShader.shader)