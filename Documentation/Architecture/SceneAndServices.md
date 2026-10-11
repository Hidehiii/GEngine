# Scene, physics, assets, and services architecture

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

Component render hooks are not equivalent to the portable presentation path:
`ImageRenderer::OnRender` has no draw implementation, and
`MeshRenderer::OnRender` builds a pipeline/material but has its draw call
disabled. The implementation gap is tracked in
[rendering TODO](../../TODO/RenderingArchitecture.md); use a layer and the
graphics/graph APIs for an exercised draw path.

## Assets, tools, UI, and diagnostics

`Mesh` holds CPU vertices/indices and `MeshLibrary` indexes named meshes;
`ModelImporter` reads FBX meshes. `Serializer` handles configuration, scene,
material, and shader-cache data. `ShaderCompiler` supplies backend shader
bytecode/reflection; file, string, stream, and hashing helpers support these
workflows. `GraphicsTool` declares point/line rendering but its methods are
currently empty. `Utils` contains GUI texture/UV and file-dialog helpers,
while `StbBuild.cpp` is an internal image-loader translation unit.

Optional `ImGuiLayer` bridges to backend UI integration. `Debug` provides
instrumentation/trace output, not frame ownership. The separate
`GEngine-Editor` application consumes engine APIs and ImGui; backend allocation
and synchronization remain within the engine.
