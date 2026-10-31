# Scene, component, and physics API

[API index](README.md) · [Scene architecture](../Architecture/SceneAndServices.md)

## GameObject base data

`Scene::CreateGameObject(name)` creates an object with base data and no attached
components. The explicit-ID overload takes a uint64_t UUID; duplicate IDs fail.
Use `GetTransform()`, `GetUUID()`, `GetName()/SetName()` and
`IsActive()/SetActive()`. Transform is a value type in
`GEngine/Object/Transform.h`, not an attachable component.
GameObject is a non-owning scene handle; it must not outlive its scene.

`Parent()` returns a GameObject handle; `SetParent(parent)` rejects cycles and
cross-scene links. `SetParent()` detaches. Relationships do not compose local
transform matrices. Scene copy remaps links; duplication preserves base data and
assigns a new UUID. DestroyGameObject is deferred until OnEndFrame and detaches
children. `GetGameObjects<>()` enumerates all objects.
`AddScript`, `RemoveScript`, `HasScript` and `GetScriptClasses` manage managed
script class names attached to the object. Script classes are dispatched either
as C# `GEngine.ScriptBehaviour` instances or as statically registered C++
`GEngine::NativeScriptBehaviour` instances. Use
`GE_REGISTER_NATIVE_SCRIPT(Type, "Namespace.Type")` in game code to register a
C++ behaviour; scene serialization stores only the class name and does not
load source files dynamically.

## Retained components

Only StaticMeshComponent, ImageComponent, CameraComponent and
DirectionalLightComponent are accepted by AddComponent, AddOrReplaceComponent,
HasComponent, GetComponent, TryGetComponent and RemoveComponent.
Transform/Attribute and native script, physics, legacy light, MeshFilter,
MeshRenderer and ImageRenderer component APIs have been removed.

`StaticMeshComponent::SetMeshAsset(const Ref<const Mesh>&)` holds a shared, immutable
in-memory mesh asset; callers replace the asset rather than mutating aliases.
`LoadMeshAsset(path, index=0)` loads a mesh .gasset and records its persistent
path/index. `GetMeshAsset()`, `GetMeshAssetPath()`, `GetMeshIndex()` expose
the current selection. SetMaterial/GetMaterial manage the material. Missing
assets/materials or invalid indices fail explicitly. Mesh/material replacement
invalidates the pipeline; component copies retain assets/materials and rebuild it.

`ImageComponent::SetImageAsset(const Ref<Texture2D>&)` and `LoadImageAsset(path)`
select a texture asset; GetImageAsset returns it. SetMaterial/GetMaterial manage
the material. Color and Tiling are per-component values. An empty image selection
uses white for a color-only quad. The material shader must declare:
GE_MATRIX_M (float4x4), GE_IMAGE_COLOR (float4), GE_IMAGE_TILING (float2),
GE_IMAGE_TEXTURE (Texture2D) and GE_IMAGE_SAMPLER (sampler).
StaticMeshComponent shaders require GE_MATRIX_M and the full Vertex input layout.
Use separate materials for objects with different per-object constants.
The separate image/sampler contract is portable: D3D12 and Vulkan use distinct
descriptors, while OpenGL internally combines them and keeps the material API
names unchanged.

CameraComponent retains its existing orthographic/perspective, primary, RenderOrder and
viewport behavior. OnViewportResize updates cameras for nonzero sizes;
MainCamera selects the primary camera. EditorCamera remains a standalone utility.

`DirectionalLightComponent` stores `Color` (`Vector4`) and `Intensity`
(`float`). `GetDirection()` returns the owning GameObject's
`Transform::Forward()`; only the color and intensity are serialized, and
loaded intensity must be finite and nonnegative. SceneRenderer selects the
first active DirectionalLight. Lit materials must declare all three constants:
GE_DIRECTIONAL_LIGHT_DIRECTION (float3), GE_DIRECTIONAL_LIGHT_COLOR (float4)
and GE_DIRECTIONAL_LIGHT_INTENSITY (float). Materials that do not declare the
complete contract are unchanged. If no active directional light exists, the
renderer supplies a white, forward-facing default. Multiple-light
accumulation, shadows and a backend global uniform buffer are not implemented.

## Recording, serialization and services

Call `SceneRenderer::Render(scene, command)` inside an open graphics pass, or
`GameObject::OnRender()` during OnPresent. SceneRenderer applies the
DirectionalLightComponent contract documented above.
ImageComponent/StaticMeshComponent also expose OnRender(command). The caller
owns targets, graph dependencies and submissions; backends retain resources
through GPU completion. No automatic camera pass, sorting or
parent-transform composition is added.

Serializer<Scene> writes GameObject/Name/Active/Transform/optional Parent,
CameraComponent, ImageComponent, StaticMeshComponent and
DirectionalLightComponent maps. ImageAsset/MeshAsset store .gasset paths;
StaticMeshComponent also stores MeshIndex. Material stores Shader and Name only;
arbitrary material constants/resources/render-state overrides remain runtime-only.
SetMeshAsset alone has no persistent submesh identity: use LoadMeshAsset before
saving it. Procedural resources without persistent paths fail save explicitly.
Unknown/removed component keys fail load and leave the original scene reference
unchanged. Legacy Attribute.IsActive is read as base metadata only; no Attribute
component is created. First-party scenes use the new keys.

Scene lifecycle no longer steps physics worlds. `Scene::OnAwake`, `OnStart`,
`OnFixedUpdate`, `OnUpdate(deltaTime)`, `OnLateUpdate`, `OnPause`, `OnResume`
and `OnEndFrame` define the current object/component/script tick. Awake/start
run once, active changes dispatch enable/disable, fixed update uses the
configured fixed timestep with a bounded accumulator, and paused scenes skip
fixed/variable/late updates. Deferred destruction dispatches OnDestroy.
Component virtual hooks include OnAwake, OnStart, OnEnable, OnDisable,
OnFixedUpdate, OnUpdate, OnLateUpdate, OnPause, OnResume and OnDestroy. A
managed or native exception stops that script instance's future updates but
still allows OnDestroy cleanup. Standalone
physics and Mono runtime services remain; Physics2DBodyDef::SetType now takes
`PhysicsBodyType::{Static,Dynamic,Kinematic}`. Contact listener hooks can be
overridden but do not dispatch scene scripts. GraphicsTool point/line helpers
remain explicitly unsupported.
Physics2D and Physics3D are built as separate native service backend modules
and linked into GEngine; they are not graphics-API backend selections.
Physics callback ordering, Transform/engine APIs, dynamic C++ source loading
and project script assembly loading remain open in
[TODO](../../TODO/SceneAndPhysics.md).

See [SceneRenderHelpers](../../Example/SceneRenderHelpers/README.md) for asset,
material, replacement, scene-roundtrip and available-backend checks, and
[GameWorld](../../Example/GameWorld/README.md) for the world-logic/script
lifecycle regression. Current evidence and remaining work live in
[TODO](../../TODO/SceneAndPhysics.md).
