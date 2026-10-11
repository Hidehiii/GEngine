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
contact callbacks. Component rendering is incomplete:
`ImageRenderer::OnRender` does not draw and `MeshRenderer::OnRender` does
not issue its draw call. Use a layer and the
[graphics API](GraphicsAPI.md) for an exercised rendering path; the gap is
tracked in [rendering TODO](../../TODO/RenderingArchitecture.md).
