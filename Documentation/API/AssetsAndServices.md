# Assets, math, tools, and service API

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

The engine Gizmo forwarding interface is planned, not available. Its scope and
acceptance checks are in the
[Gizmo forwarding TODO](../../TODO/Build/ThirdPartyDependencies.md#planned-engine-owned-gizmo-forwarding-interface).
It will use engine-owned types rather than expose ImGuizmo or draw-list types;
existing public API signatures remain unchanged by this documentation update.

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
