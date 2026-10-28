# Scene, component, and physics API

[API index](README.md) · [Scene architecture](../Architecture/SceneAndServices.md)

## Scene and objects

`Scene::CreateGameObject(name)` creates an entity with `Attribute`,
`Transform`, and `NativeScript` components. `GameObject` is a handle into
its scene, not an owning copy. Keep the scene alive while using object handles.
`AddComponent<T>`, `AddOrReplaceComponent<T>`, `HasComponent<T>`,
`GetComponent<T>`, `TryGetComponent<T>`, and `RemoveComponent<T>` operate
on that registry. `Scene::DestroyGameObject` queues deletion until
`OnEndFrame`; `DuplicateGameObject` and `Scene::Copy` copy components in
`AllComponents`.

An application or editor that owns a scene calls `OnAwake` before `OnStart`,
then `OnUpdate`, `OnLateUpdate`, and `OnEndFrame` as appropriate. The engine
runtime does not call these automatically. `OnViewportResize` updates camera
viewport sizes. `MainCamera` and `MainDirectionalLight` select primary
components.

## Components and physics

`Transform` stores position, rotation, and scale and computes model matrices.
`Camera` has orthographic/perspective settings; `EditorCamera` is the
editor-oriented derivative. Other components include `ImageRenderer`,
`MeshFilter`, `MeshRenderer`, `DirectionalLight`, `BoxCollider2D`,
`CircleCollider2D`, `RigidBody2D`, `BoxCollider`, and `RigidBody`.

`Scene::OnAwake` creates Box2D and PhysX worlds and builds 2D bodies/fixtures
from existing rigid-body/collider components. `OnUpdate` performs fixed 2D
steps and physics hooks. The scene fixed-update code does **not** call
`Physics3DWorld::Simulate`: a 3D world/body wrapper exists, but automatic 3D
simulation is not a supported scene behavior. The runtime must have PhysX
enabled before scene 3D-world creation. See
[scene/physics TODO](../../TODO/SceneAndPhysics.md).

`NativeScript` holds C++ `ScriptableObject` instances with lifecycle and
contact callbacks. Set a material on each `ImageRenderer` and `MeshRenderer`
before drawing; `MeshRenderer` also needs a nonempty `MeshFilter`. Call
`GameObject::OnRender()` during a layer's `OnPresent`: it dispatches both
components through the portable presentation command buffer. ImageRenderer
draws a color quad and writes `GE_MATRIX_M` and `GE_IMAGE_COLOR` material
constants; MeshRenderer draws the indexed mesh and writes `GE_MATRIX_M`.
Give each component its own material when it has per-object constants.
For an offscreen render graph pass, include `GEngine/Renderer/SceneRenderer.h`
and call `SceneRenderer::Render(*scene, command)` from the graphics recording
callback. This records active objects' image/mesh components into that pass;
it neither acquires/presents nor executes scene update/physics. The target,
resource dependencies and command submission belong to the caller. Individual
components provide `OnRender(const Ref<CommandBuffer>&)` as well; null commands
throw `std::invalid_argument`. Existing no-argument calls remain compatible.
Projection/camera constants remain the material's responsibility; the shared
recorder does not implement automatic camera/light rendering or sorting.
`ImageRenderer::m_Texture` drawing and `GraphicsTool::Init/RenderPoint/RenderLine`
currently throw `std::logic_error` as unsupported paths. Missing materials
also fail explicitly. The [rendering TODO](../../TODO/RenderingArchitecture.md)
tracks manual visual and backend validation still in progress.
The [SceneRenderHelpers example](../../Example/SceneRenderHelpers/README.md)
is the new sample application for the retained
component paths. It selects a backend in `Config.ini`, supports
`GENGINE_SCENE_HELPER_FRAME_LIMIT=120` and optional
`GENGINE_SCENE_HELPER_SYNTHETIC_RESIZE=1` on D3D12 only, and documents manual
visibility, resize, and shutdown checks. Vulkan/OpenGL ignore this diagnostic;
real resize remains a manual check. The final Debug and Release runs on D3D12,
OpenGL, and Vulkan each completed
120 frames with empty stderr and replacement/exit logs. This confirms command
recording and shutdown, not visible output, real window resize, or long-run
memory stability. The user subsequently reported no problem with the manual
test; this is user-observed evidence, not an agent-captured per-backend log or
long-run memory measurement.
