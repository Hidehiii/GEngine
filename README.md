# GEngine

A game engine.

Its rendering is developed based on the OpenGL graphics API and Vulkan, the 2D physics is based on the Box2D library, and the 3D physics is based on Nvidia's PhysX library.

### Dependence
##### [VulkanSDK](https://www.lunarg.com/vulkan-sdk/)
Version: windows x64 

All components should be installed.
##### [.Net Framework](https://dotnet.microsoft.com/zh-cn/download/visual-studio-sdks?cid=getdotnetsdk)
Version: .Net Framewor 4.7.2 Developer Pack

### Start
run ```Scripts/GenerateProject.bat``` and everything of project would be setted up.

### Documentation
#### Shader(.glsl)

[Explame shader](Example/Shader/ExampleShader.glsl)

##### Symbol
[#Name XXX]()

XXX stands for the name of this Shader.

[#Blend Type Src Dest]()

[#Blend ColorType SrcColor DestColor AlphaType SrcAlpha DestAlpha]

Type indicates the blend type.

Type : (None, Alpha, Additive, Multiply)

Src represents the source target.

Src : (SrcAlpha, DstAlpha, SrcColor, DstColor, OneMinusSrcAlpha, OneMinusDstAlpha, OneMinusSrcColor, OneMinusDstColor, One, Zero)

Dest : represents the target object.

Dest : (SrcAlpha, DstAlpha, SrcColor, DstColor, OneMinusSrcAlpha, OneMinusDstAlpha, OneMinusSrcColor, OneMinusDstColor, One, Zero)

[#DepthWrite X]()

Indicates whether to enable DepthWrite.

X : (On, 1, True, Off, 0, False)

[#DepthTest X]()

Indicates whether to enable DepthTest.

X : (On, 1, True, Off, 0, False)

[#Properties]()

[PropertiesType PropertiesName;]()

[#EndProperties]()

Used to mark the following statement as an attribute.

Write code of the properties between ```#Properties``` and ```#EndProperties```.

Each line of the statement should be written like this:

PropertiesType PropertiesName; 

PropertiesType : (float, int, color, vector, sampler2D, storageImage2D, storageBuffer, samplerCubeMap)

Do not forget to call #EndProperties at the end.

[#Cull Type]

Indicates the cull mode.

Type : (None, Front, Back)

[#Type Vertex]()

To identify the code block bit vertex shader below.

[#Type Fragment]()

To identify the code block bit fragment shader below.

Vertex and Fragment shader write with reference vulkan glsl.

[#Type Compute]()

To identify the code block bit compute shader below.

All shader should include the Core.glsl file.