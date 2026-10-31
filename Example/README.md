# Examples

Each example directory contains a matching .gproject. Open it with File > Open
project in the Qt editor, or pass its path as the editor's first argument.
Project > Project settings selects the default scene using a project-relative path.

| Project | Default editor scene |
| --- | --- |
| [DeferredRender](DeferredRender/DeferredRender.gproject) | Empty DeferredRender editor scene |
| [FrameGraphTriangle](FrameGraphTriangle/FrameGraphTriangle.gproject) | Red and cyan color quads |
| [RenderPipeline](RenderPipeline/RenderPipeline.gproject) | Empty RenderPipeline editor scene |
| [GpuDriven](GpuDriven/GpuDriven.gproject) | Empty GpuDriven editor scene |
| [GameWorld](GameWorld/GameWorld.gproject) | Empty GameWorld editor scene |
| [InputSystem](InputSystem/InputSystem.gproject) | Empty InputSystem editor scene |
| [SceneRenderHelpers](SceneRenderHelpers/SceneRenderHelpers.gproject) | Empty SceneRenderHelpers editor scene |
| [ShaderCompile](ShaderCompile/ShaderCompile.gproject) | Empty ShaderCompile editor scene |
| [Triangle](Triangle/Triangle.gproject) | Empty Triangle editor scene |

The default file in each project is Assets/Scenes/Default.gscene. The nine empty
scenes are explicit editor starting points; the examples' native C++ render/input/
shader workloads continue to run through their existing application entry points.
Runtime resources in these projects use the `.gasset` YAML envelope; scenes use
`.gscene`.
Opening a project does not run that native workload. Historical serialized scenes
with cameras, textures or physics components remain available, but those components
are not supported by the current color-scene Worker loader. A successful empty
scene load shows a clear viewport and its filename, with the scene-loaded log in
Console. Missing defaults or Worker errors indicate a failed load.

No engine rebuild is needed for these project descriptors/scenes. Build the current
Qt Host/EngineWorker first; see [editor instructions](../GEngine-Editor/README.md).
