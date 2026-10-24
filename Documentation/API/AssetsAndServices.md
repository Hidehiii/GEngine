# Assets, math, tools, and service API

新编辑器的文档/命令/Worker 协议见 [Qt 编辑器契约](QtEditor.md)。本次仅
同步设计入口，当前 Scene、Serializer、ImGuiLayer 及其他引擎签名不变。

UI 选型修订：Qt 6 Widgets 用于未来 EditorHost，游戏/Worker 内继续使用
EnableImGui、ImGuiLayer 和 OnImGuiRender；现有引擎接口不引入 Qt 类型。
Qt 迁移尚未实现，现有编辑器 GizmoController 仅作过渡，见[接口设计](Editor.md)。

编辑器 Worker 协议为计划接口：Host 不加载引擎，跨进程只传 DTO 与已完成
图像，不传 Scene 指针或 GPU handle。当前 API 尚未提供该隔离能力。

编辑器重设计的[接口契约提案](Editor.md)与[实施 TODO](../../TODO/EditorArchitecture.md)
已单独记录。当前公共接口保持不变，文档/命令/运行会话等名称尚不是可调用 API。

The editor final presentation quad uses GUIUtils::GetUV0() as its top-left
attachment UV and GetUV1() as its bottom-right UV. No public API changes;
scene/game ImGui image UVs continue using the same endpoints.

Editor repair in progress: GUIUtils texture IDs now support D3D12 as well as
OpenGL/Vulkan. Obtain IDs during OnImGuiRender and do not cache them between
frames: descriptor ownership is tied to the current completed/reused frame slot.
The editor uses Config getters; backend/frame-count changes require editing
Config.ini and restarting. VSync can be changed for the active window. Managed
scripting requires both the core assembly and Mono mscorlib runtime in the
editor working directory; it is disabled when either is missing.
See [editor tracking](../../TODO/Build/ThirdPartyDependencies.md).

`GEngine-Editor/RunEditor.bat [Debug|Release]` locates a VS2022 CMake/Premake
executable and launches from the editor resource directory. Set
`GENGINE_EDITOR_FRAME_LIMIT=120` for bounded verification; zero/unset is unlimited.
Preview color/vertex-color rendering is available, while textured/lit/custom
material preview and camera MSAA are explicitly unsupported.

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

There is no engine-exported Gizmo API. The editor-owned GizmoController now
wraps pristine upstream ImGuizmo, without exposing third-party types. Its
operations and migration checks are described in the [editor API](Editor.md) and
[Gizmo forwarding TODO](../../TODO/Build/ThirdPartyDependencies.md#planned-engine-owned-gizmo-forwarding-interface).
ImGuiLayer no longer includes ImGuizmo.h or begins its frame; applications that
use gizmos must own that integration explicitly. Other engine signatures are unchanged.

The subsequent dependency upgrade changes `GUIUtils::GetTextureID` to return
`uint64_t`, matching current ImGui texture identifiers. Image buttons require an
explicit string ID. OpenGL and Vulkan texture IDs are supported; other backends
report an unsupported operation. Upgrade verification remains tracked in the
[dependency TODO](../../TODO/Build/ThirdPartyDependencies.md).

When `EnableImGui` is true, the runtime pushes an `ImGuiLayer` overlay.
Layers implement `OnImGuiRender`; `GUIUtils` supplies texture IDs and UVs
for UI integration. `FileDialogs::OpenFile/SaveFile` return an empty string
on cancel. `ScriptEngine::Init/Shutdown` bootstrap Mono when scripting is
enabled, but the current implementation loads a fixed script-core assembly
rather than exposing general managed-component registration. For the exposed
scene scripting path, use C++ `NativeScript` (see
[Scene and physics API](SceneAndPhysics.md)).

`Instrumentor` in `<GEngine/Debug/Instrumentation.h>` writes trace events
for profiling sessions. Backend validation is separate; see
[D3D12 diagnostics](../Diagnostics/D3D12Diagnostics.md). These tools do not
replace backend-specific runtime checks.
