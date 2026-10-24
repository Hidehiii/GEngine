# Scene, physics, assets, and services architecture

编辑器提案新增进程隔离：Host 拥有独立文档与 UI，Worker 拥有引擎 Scene、
预览及运行实例，经有界异步协议同步；当前同进程引擎 API 未改变。

新的编辑器模块划分见[编辑器架构提案](Editor.md)，实施工作由
[编辑器 TODO](../../TODO/EditorArchitecture.md) 管理。此设计未改变当前 Scene、
服务所有权或运行行为；现有实现与验证范围仍以下文及修复 TODO 为准。

Editor presentation maps fullscreen quad UVs through GUIUtils attachment-origin
endpoints. OpenGL samples the top at V=1; Vulkan/D3D12 sample the top at V=0.
This corrects final UI inversion without changing scene/camera coordinates or
backend viewport policy. Visual confirmation remains open in the editor TODO.

Editor repair in progress (2026-10-23): the editor records its scene/game preview
targets using portable command buffers and presents the ImGui target with a modern
shader. Mesh vertex colors and image colors are previewed; lit/custom materials
are explicitly outside this preview. The grid uses ImGuizmo drawing. UI texture
descriptors and retained resources are recycled per completed frame slot; D3D12
has a native UI texture adapter and Vulkan releases descriptor sets before its
UI pool shuts down. Scene::GetGameObjects uses entity values from EnTT iteration.
Build/runtime evidence is owned by the [editor TODO](../../TODO/Build/ThirdPartyDependencies.md).

The preview also exercises combined-image-sampler reflection and Vulkan command
buffer ownership. Graphics/compute pipelines are retained while recording and
submitted; their buffers/materials cannot disappear before GPU completion. The
editor restores a default dock layout only when no saved dock node exists.
Debug/Release editor builds and bounded runs passed on all three backends; visible
interaction and measured long-run resource behavior remain unverified.

[Architecture overview](Architecture.md) · [Scene API](../API/SceneAndPhysics.md) ·
[Assets and services API](../API/AssetsAndServices.md)

## Scene and component ownership

`Scene` owns an `entt` registry. `Scene::CreateGameObject` creates an entity
with `Attribute`, `Transform`, and `NativeScript`; `GameObject` stores the
entity handle plus a non-owning scene pointer. It must not outlive its scene.
`Scene::Copy` and duplication copy the component set in `AllComponents`.
`DestroyGameObject` defers removal to `OnEndFrame`; scene resize updates
camera components. Scene/component lifecycle is called by an application or
editor, not by the engine runtime automatically.

Components cover attributes, transforms, cameras/editor camera, directional
lights, image/mesh render data, native scripts, and 2D/3D colliders and rigid
bodies. `Math` provides the vector, matrix, quaternion, projection, and
transformation operations used across rendering and physics.

## Physics and scripting

`Scene::OnAwake` creates Box2D and PhysX worlds, attaches the 2D contact
listener, and builds 2D bodies/fixtures for existing rigid-body/collider
components. Its fixed-update loop steps the **2D** world and invokes physics
hooks; `Scene::OnFixedUpdate` does not step the 3D world. The runtime initializes
global PhysX only when `EnablePhysics` is true. The 3D world/body wrappers
exist, but automatic scene-driven 3D simulation remains an open follow-up in
[scene and physics tracking](../../TODO/SceneAndPhysics.md).

`NativeScript` owns C++ `ScriptableObject` instances with lifecycle and contact
callbacks. Separately, `ScriptEngine` bootstraps a Mono domain and a fixed
script-core assembly when `EnableScripting` is true; it is not yet a general
managed-component attachment interface.

`GameObject::OnRender` now dispatches image and mesh renderer components. Each
uses a caller-provided material and the active portable presentation command
buffer, so a layer calls it during `OnPresent`, not during its offscreen
`OnRender`. ImageRenderer owns a quad pipeline; MeshRenderer invalidates its
pipeline on material changes and rebuilds it on mesh changes or the next draw.
Textured images and GraphicsTool point/line paths explicitly report unsupported
instead of being silent no-ops.
The [SceneRenderHelpers example](../../Example/SceneRenderHelpers/README.md)
is a separate application so its runtime and materials
are not coupled to `FrameGraphTriangle`'s GPU graph workload. It owns two
scene objects, calls their render hooks during presentation, replaces a
material and mesh after frame boundaries, and can request synthetic resize.
Synthetic resize is D3D12-only and one-shot at each selected frame: Vulkan's
swap-chain extent must match its native surface, so a synthetic smaller extent
without a real window resize can keep acquisition from progressing. After the
one-shot fix, Debug and Release runs on D3D12, OpenGL, and Vulkan
each reached 120 frames and exited 0. D3D12 alone performed two synthetic
resizes per run. Visible pixels, real resize, and long-run memory remain
unverified by the agent in the [rendering TODO](../../TODO/RenderingArchitecture.md).
The user subsequently reported that the requested manual test had no problem;
per-backend output and long-run memory measurements were not supplied.

## Assets, tools, UI, and diagnostics

`Mesh` holds CPU vertices/indices and `MeshLibrary` indexes named meshes;
`ModelImporter` reads FBX meshes. `Serializer` handles configuration, scene,
material, and shader-cache data. `ShaderCompiler` supplies backend shader
bytecode/reflection; file, string, stream, and hashing helpers support these
workflows. `GraphicsTool` declares point/line rendering but its methods now
throw an unsupported-operation error. `Utils` contains GUI texture/UV and
file-dialog helpers,
while `StbBuild.cpp` is an internal image-loader translation unit.

Optional `ImGuiLayer` bridges to backend UI integration. `Debug` provides
instrumentation/trace output, not frame ownership. The separate
`GEngine-Editor` application consumes engine APIs and ImGui; backend allocation
and synchronization remain within the engine.

Planned only: an engine-owned Gizmo forwarding boundary will keep ImGuizmo
implementation and draw-list/context handling internal, while the editor consumes
engine types. The scope and acceptance checks are owned by the
[Gizmo forwarding TODO](../../TODO/Build/ThirdPartyDependencies.md#planned-engine-owned-gizmo-forwarding-interface).
No ownership, backend, or runtime behavior changes are implemented by this plan.

Dependency upgrade in progress: ImGui v1.93.0-docking uses backend-managed font
texture updates. D3D12 supplies reusable SRV allocation callbacks; Vulkan supplies
its render pass through pipeline info. A first-party compatibility translation
unit adapts the legacy ImGuizmo mouse-capture call. The forwarding interface
above remains planned. See the [dependency TODO](../../TODO/Build/ThirdPartyDependencies.md).

The UI backend base has a virtual destructor so renderer shutdown executes before
context destruction. ImGuiLayer retains window-manager selection for shutdown
without querying the Application runtime being destroyed. D3D12 UI command
buffers use tracked queue submission and SRV descriptors are recycled by callbacks.
