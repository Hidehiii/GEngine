# Scene, physics, assets, and services architecture

## GAsset envelope and scene extension

First-party runtime resources use `.gasset`, and scenes use `.gscene`. A
version-1 GAsset is a YAML envelope with `schemaVersion`, `type`, `format`,
`payloadEncoding` and `payload`. Text shader and shader-include payloads are
stored as UTF-8 YAML scalars; texture, mesh and font payloads use standard YAML
binary Base64. The envelope is deliberately not a claim that every resource has
a mature engine-owned CPU schema: its payload is still the declared source/import
format, while the full typed resource-data design remains tracked in
[Scripting and packaging TODO](../../TODO/ScriptingAndPackaging.md).

`GAsset.cpp` validates the extension, YAML map, exact field set, schema version,
identifier characters, requested type and payload encoding before returning
owned bytes. ImageComponent, mesh, font, shader and DXC include loaders unwrap the
payload and retain their existing backend ownership rules. `.gscene` remains
the existing scene YAML with a scene-specific extension; it is not wrapped in
GAsset. Legacy raw source extensions remain loader compatibility inputs, but
first-party examples and editor runtime resources use the two-extension
convention.
`ModelImporter` explicitly destroys the FBX importer, scene and IO settings;
the FBX manager is owned only by its RAII holder, including failed imports.
`Mesh` copies expose the CPU vertices
and indices while omitting the importer-only deduplication map, and `Vertex`
uses a deterministic component hash.
Binary GAsset payloads currently use YAML Base64, which inflates large font and
texture files by about 33%; a compact binary envelope remains a future
optimization.

当前编辑器采用 [Qt Host/Worker 架构](QtEditor.md)。2026-10-25 已移除旧 ImGui
编辑器、私有面板与 GizmoController；旧实现及验证记录保留在 Git/TODO 历史。
Qt Host 负责 UI/草稿，独立 GEngine Worker 提供真实 GPU 预览和日志。
完整 Scene 编辑、Play、输入与 Qt Gizmo 仍待实施。当前 Scene/Serializer、游戏
ImGui 与引擎 EditorCamera 的接口和所有权不受本次删除影响。

## Scene and component ownership

Only `StaticMeshComponent`, `ImageComponent`, `CameraComponent` and the retained
`DirectionalLightComponent` are attachable scene components. `GameObject`
remains a non-owning registry handle. The scene's
`GameObjectData` owns UUID, name, active state, Transform values and a parent
entity handle; Transform no longer inherits Component. Keep the scene alive
while using handles. Removed components include Attribute, NativeScript,
MeshFilter/MeshRenderer/ImageRenderer, legacy light components, rigid bodies
and colliders.

StaticMeshComponent owns a shared immutable CPU Mesh asset and a material. Replacing
either invalidates its GPU pipeline; copying retains those owners but rebuilds
the destination pipeline. ImageComponent owns a sampled Texture2D asset and material,
and records a position/UV quad with color and tiling. Missing images use a white
texture for color-only scenes. CameraComponent projection and selection behavior remain;
EditorCamera remains a camera utility, not an additional attachable type.
DirectionalLightComponent is the first retained light type. Its direction is
read from the owning GameObject's `Transform::Forward()`, while Color and
Intensity are component data. SceneRenderer selects the first active
DirectionalLight and writes the direction/color/intensity into scene materials
that declare the three `GE_DIRECTIONAL_LIGHT_*` constants; materials without
that complete contract are unchanged. With no active directional light it uses
a white forward-facing default. This milestone has no multiple-light
accumulation, shadows, backend-specific light state, or global uniform buffer.

Scene copy duplicates base data, reattaches components and remaps parent handles.
Duplication assigns a new UUID. Deferred deletion detaches children and ignores
repeated deletion requests. Hierarchy links preserve relationships; model matrices
remain local Transform matrices (no new parent-transform composition).
Scene lifecycle no longer creates or steps physics worlds, or dispatches native
scripts. Standalone Box2D/PhysX and Mono runtime services remain optional and
independent; Physics2D and Physics3D are now separate native service backend
modules linked into GEngine. Physics2DBodyDef uses PhysicsBodyType instead of
a removed component enum. Standalone contact listener hooks no longer dispatch
scene scripts.
Scene update behavior now has a bounded world-logic pipeline: awake and start
run once, active changes dispatch enable/disable, variable update runs a
bounded fixed-update accumulator followed by object/component update, and late
update, pause/resume and deferred destruction have explicit phases. GameObject
owns object-level awake/start state, stores script class names and forwards
phases to retained components. Serializer<Scene> persists those class names as
the `Scripts` sequence. ScriptEngine owns managed `ScriptBehaviour` instances
per scene/object/class, updates a minimal C# GameObject wrapper, dispatches the
same lifecycle phases, stops future dispatch after a managed or native
exception, and still calls OnDestroy for cleanup. Game-side C++ behaviours
derive from `NativeScriptBehaviour` and are statically registered by class
name; they are not dynamically loaded source files. Physics stepping/contact
ordering, Transform/engine APIs, project assemblies and editor script
authoring remain open in [SceneAndPhysics](../../TODO/SceneAndPhysics.md).

Scene serialization writes base data plus CameraComponent,
StaticMeshComponent, ImageComponent and DirectionalLightComponent, including
asset paths, mesh index, each material's shader path/name, and light
color/intensity. Arbitrary runtime
material property overrides are not serialized. In-memory procedural meshes or
textures without persistent asset identities fail serialization explicitly.
Removed/unknown component keys fail loading; failed loads preserve the destination
scene by constructing a candidate first. Legacy Attribute.IsActive is accepted
only as a base-data input adapter; new output writes Active directly. Shipped
scenes have been migrated, removing historical physics/circle-renderer data.
Legacy physics-demo filenames do not imply retained simulation.

SceneRenderer records active ImageComponent/StaticMeshComponent components into an already-open
graphics pass and applies the selected DirectionalLightComponent contract above;
GameObject::OnRender is the presentation adapter. Command/resource
retention remains in backends: replacing CPU references does not imply GPU
completion. No native handles, queue semantics or backend APIs changed.
ImageComponent shaders explicitly expose the sampled-image binding contract documented
in the [component API](../API/SceneAndPhysics.md). Missing bindings fail before
rendering rather than silently dropping required work. D3D12 and Vulkan bind
the reflected Texture2D and sampler resources separately. OpenGL translates
that reflection into a SPIRV-Cross combined image sampler and maps both
material bindings to the image's texture unit; the component-facing contract
remains backend-neutral.

The [SceneRenderHelpers example](../../Example/SceneRenderHelpers/README.md)
draws a textured quad and a direction-lit indexed mesh, replaces
materials/meshes/images, changes the light at frame 45 and runs base-data,
copy/duplicate, hierarchy, serialization and malformed-resource checks.
Current verification and remaining manual/backend checks are tracked in
[SceneAndPhysics](../../TODO/SceneAndPhysics.md).

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
