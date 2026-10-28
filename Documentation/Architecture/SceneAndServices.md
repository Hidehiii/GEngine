# Scene, physics, assets, and services architecture

当前编辑器采用 [Qt Host/Worker 架构](QtEditor.md)。2026-10-25 已移除旧 ImGui
编辑器、私有面板与 GizmoController；旧实现及验证记录保留在 Git/TODO 历史。
Qt Host 负责 UI/草稿，独立 GEngine Worker 提供真实 GPU 预览和日志。
完整 Scene 编辑、Play、输入与 Qt Gizmo 仍待实施。当前 Scene/Serializer、游戏
ImGui 与引擎 EditorCamera 的接口和所有权不受本次删除影响。

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

共享 Mono SDK、开发/发布路径、C# 编译与 Worker 重启的目标设计见
[C# 与游戏发布架构](ManagedScripting.md)。`RuntimeSpecification` 现在提供
`ScriptRuntimeRoot` 与 `ScriptCoreAssembly`，`EngineRuntime` 在切换 cwd 前解析
相对路径并传给 `ScriptEngine`；RuntimeId/清单/发布模式仍未实施。

`Scene::OnAwake` creates Box2D and PhysX worlds, attaches the 2D contact
listener, and builds 2D bodies/fixtures for existing rigid-body/collider
components. Its fixed-update loop steps the **2D** world and invokes physics
hooks; `Scene::OnFixedUpdate` does not step the 3D world. The runtime initializes
global PhysX only when `EnablePhysics` is true. The 3D world/body wrappers
exist, but automatic scene-driven 3D simulation remains an open follow-up in
[scene and physics tracking](../../TODO/SceneAndPhysics.md).

`NativeScript` owns C++ `ScriptableObject` instances with lifecycle and contact
callbacks. Separately, `ScriptEngine` bootstraps a Mono domain and the configured
script-core assembly when `EnableScripting` is true; it is not yet a general
managed-component attachment interface.

`SceneRenderer::Render(Scene&, command)` records active ImageRenderer and
MeshRenderer components into the caller's already-open graphics pass, using
their existing material/pipeline implementation. It owns neither scene nor
target and does not start scripts, physics, or a camera pass. The caller keeps
the scene alive through recording; backend command retention governs submitted
GPU resources. Materials still determine camera/projection semantics.
Components also expose `OnRender(command)`; null commands fail explicitly.
The no-argument methods remain presentation compatibility adapters.
Worker scene preflight uses UTF-8 paths through FileSystemHelper; IsDocument
now makes the same native UTF-8 conversion as the subsequent resource read.

`GameObject::OnRender` dispatches image and mesh renderer components. Each
uses a caller-provided material and the active portable presentation command
buffer, so a layer calls it during `OnPresent`, not during its offscreen
`OnRender`. ImageRenderer owns a quad pipeline; MeshRenderer invalidates its
pipeline on material changes and rebuilds it on mesh changes or the next draw.
Textured images and GraphicsTool point/line paths explicitly report unsupported
instead of being silent no-ops.
The [SceneRenderHelpers example](../../Example/SceneRenderHelpers/README.md)
is a separate application so its runtime and materials
are not coupled to `FrameGraphTriangle`'s GPU graph workload. It owns two
scene objects, alternates shared scene recording with compatibility hooks during presentation, replaces a
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

Optional `ImGuiLayer` bridges engine/game UI to backend integration. `Debug`
provides instrumentation, not frame ownership. Qt Host does not link GEngine or
ImGui; its owned engine worker handles rendering. The old editor/GizmoController
has been removed, and ImGuizmo has no remaining build consumers.

Dependency upgrade history: ImGui v1.93.0-docking uses backend-managed font
texture updates. D3D12 supplies reusable SRV allocation callbacks; Vulkan supplies
its render pass through pipeline info. ImGuizmo's former compatibility translation
unit is removed: the pinned upstream revision handles current ImGui APIs without
vendor patches. See the [dependency TODO](../../TODO/Build/ThirdPartyDependencies.md).

The UI backend base has a virtual destructor so renderer shutdown executes before
context destruction. ImGuiLayer retains window-manager selection for shutdown
without querying the Application runtime being destroyed. D3D12 UI command
buffers use tracked queue submission and SRV descriptors are recycled by callbacks.
