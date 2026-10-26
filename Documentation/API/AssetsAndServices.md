# Assets, math, tools, and service API

新编辑器的文档/命令/Worker 协议见 [Qt 编辑器契约](QtEditor.md)。本次仅
同步设计入口，当前 Scene、Serializer、ImGuiLayer 及其他引擎签名不变。

UI 选型修订：Qt 6 Widgets 用于未来 EditorHost，游戏/Worker 内继续使用
EnableImGui、ImGuiLayer 和 OnImGuiRender；现有引擎接口不引入 Qt 类型。
Qt Host/诊断及真实引擎预览协议已实现，场景迁移未实现；旧编辑器与 GizmoController 已删除，见[当前 E0.1/E0.2 契约](QtEditor.md)。通用引擎导出签名保持不变。

编辑器 Worker 的完整场景协议仍为计划接口；当前预览 Host 不加载引擎，跨进程
只传控制消息与已完成图像，不传 Scene 指针或 GPU handle。

编辑器重设计的[接口契约提案](Editor.md)与[实施 TODO](../../TODO/EditorArchitecture.md)
已单独记录。当前公共接口保持不变，文档/命令/运行会话等名称尚不是可调用 API。

GUIUtils texture IDs remain available for engine/game ImGui. Descriptor IDs are
frame-scoped and must not be cached between frames. The legacy editor and its
Mono deployment/launcher have been removed. Use `RunQtEditor.bat [Debug|Release]`
for the Qt Host; its preview worker currently disables managed scripting.

[API index](README.md) · [Service architecture](../Architecture/SceneAndServices.md)

## Math and CPU data

`Vector2/3/4`, `Matrix2x2/3x3/4x4`, `Quaternion`, and `Math` provide
projection/transform, quaternion conversion, normalization, numeric helpers,
and degree/radian conversion. `Buffer`, `UUID`, `AABB`, `Log`, and
`Timer` are summarized in the [runtime API](Runtime.md). Check the public
headers for exact overloads and ownership.

## Assets and tools

`Mesh` contains CPU vertex/index data, while `MeshLibrary` stores named
meshes. `ModelImporter::LoadMesh(path)` reads FBX meshes for later retrieval.
`Serializer` has scene, config, material, and shader-cache specializations;
it is not a blanket serialization implementation for arbitrary `T`.
`StreamReader` and `StreamWriter` are lower-level binary-stream interfaces.
`FileSystemHelper` and `StringHelper` provide file/path and string
operations. `ShaderCompiler` builds bytecode/reflection for backend shader
code; normal applications should start with `Shader::Create(path)` and the
[shader authoring guide](ShaderAuthoring.md).

`OpenSSLTool` exposes MD5/SHA256 and binary/string helpers when the runtime
service is enabled. `GraphicsTool::RenderPoint/RenderLine` are declared but
have empty implementations; do not rely on them for drawing.

## ImGui, scripting, utilities, and diagnostics

Mono 路径与增量打包的[拟议契约](ManagedScripting.md)已独立记录。
`RuntimeSpecification::ScriptRuntimeRoot/ScriptCoreAssembly` 提供显式路径，
`ScriptEngine::Init` 返回初始化是否成功；RuntimeId/清单/发布模式仍未实施。

There is no engine-exported Gizmo API. The legacy editor-owned GizmoController
has been removed; Qt tools remain planned in the [Qt editor API](QtEditor.md).
Engine/game ImGui and other engine signatures are unchanged.

The subsequent dependency upgrade changes `GUIUtils::GetTextureID` to return
`uint64_t`, matching current ImGui texture identifiers. Image buttons require an
explicit string ID. OpenGL and Vulkan texture IDs are supported; other backends
report an unsupported operation. Upgrade verification remains tracked in the
[dependency TODO](../../TODO/Build/ThirdPartyDependencies.md).

When `EnableImGui` is true, the runtime pushes an `ImGuiLayer` overlay.
Layers implement `OnImGuiRender`; `GUIUtils` supplies texture IDs and UVs
for UI integration. `FileDialogs::OpenFile/SaveFile` return an empty string
on cancel. `ScriptEngine::Init/Shutdown` bootstrap Mono when scripting is
enabled, but the current implementation loads the configured script-core assembly
rather than exposing general managed-component registration. For the exposed
scene scripting path, use C++ `NativeScript` (see
[Scene and physics API](SceneAndPhysics.md)).

`Instrumentor` in `<GEngine/Debug/Instrumentation.h>` writes trace events
for profiling sessions. Backend validation is separate; see
[D3D12 diagnostics](../Diagnostics/D3D12Diagnostics.md). These tools do not
replace backend-specific runtime checks.
