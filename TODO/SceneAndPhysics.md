# Scene and physics follow-ups

- [x] Split the standalone physics services into native backend modules
      (2026-10-31, user authorized). Scope: separate Physics2D and Physics3D
      source sets and their Box2D/PhysX link dependencies into named static
      backend modules for both CMake and Premake, while keeping the public
      Physics2D/Physics3D headers and runtime behavior unchanged. The common
      GEngine target should link the modules rather than compile their sources
      directly. Acceptance: both generators produce distinct
      `GEngine-Physics2DBackend` and `GEngine-Physics3DBackend` targets;
      Debug/Release engine builds pass; a focused runtime check initializes and
      steps both physics services and shuts down normally; build documentation
      records that these are native service backends, not graphics APIs.
      Implemented: CMake and Premake now define `GEngine-Physics2DBackend` and
      `GEngine-Physics3DBackend` static modules, exclude their sources from the
      common GEngine target, and link the modules into GEngine. Box2D and PhysX
      dependencies are owned by the backend modules; a common link anchor keeps
      their exported symbols available through GEngine. GameWorld performs a
      Box2D step and a PhysX init/shutdown smoke check.
      Verification: CMake and Premake Debug/Release GEngine/GameWorld builds and
      runs passed.

- [x] Component consolidation (2026-10-29, user authorized): at that point only
  StaticMesh, Image and the existing Camera remained attachable. Move transform, UUID, name
  and active state to GameObject base data. StaticMesh owns a shared mesh asset
  and material; Image owns an image asset and material with actual sampled draws.
  Remove MeshFilter/MeshRenderer/ImageRenderer, script/physics/light components
  and scene-driven simulation/contact dispatch; the later DirectionalLight item
  reintroduces one retained light type. Keep standalone runtime services
  out of this removal. Update scene copy/duplication/serialization, worker,
  affected examples, build discovery and public documentation. Reject removed
  scene component keys explicitly; migrate shipped scenes to the retained schema.
  Acceptance: affected engine/example Debug+Release builds, focused object data,
  asset/material copy/replace and scene roundtrip checks, available-backend draws,
  explicit untested cases and synchronized Architecture/API/example documentation.
  Naming correction: retained classes and scene keys use StaticMeshComponent,
  ImageComponent and CameraComponent. Camera behavior remains unchanged.
  Implementation is present across components, base data, scene serialization,
  Worker, examples and documentation. Final verification (2026-10-31): the
  failed FBX import path now leaves manager destruction solely to its
  unique_ptr owner, and malformed input is rejected without replacing the
  existing mesh owner. The OpenGL backend now builds SPIRV-Cross combined
  image samplers for separate Texture2D/sampler reflection and binds both to
  the image unit; Vulkan no longer rejects a separate sampler during material
  update. CMake VS2022 Debug and Release builds of GEngine,
  SceneRenderHelpers and GEngine-EditorEngineWorker passed with the installed
  Mono SDK. Six 120-frame runs (OpenGL, Vulkan, D3D12 in Debug and Release)
  exited 0, passed startup object/copy/scene/malformed-FBX checks, logged
  material/mesh/image replacement, and shut down normally. Logs are
  build/component-backend-*.log and build/component-fbx-lifetime-*.log.
  Remaining limitations: manual visible checks, real window resize/
  minimize/restore, long-run memory behavior, and a fresh Premake
  regeneration/build remain unverified and are tracked separately.

- [~] Add DirectionalLightComponent (2026-10-31, user authorized). Scope: add
  a retained scene component whose direction is derived from the owning
  GameObject transform and whose color/intensity are explicit serialized data.
  The first bounded rendering milestone selects the first active directional
  light in SceneRenderer and writes standard `GE_DIRECTIONAL_LIGHT_*` constants
  into scene materials that declare them; materials without those constants are
  unchanged. Use a no-light white default so unlit shaders do not turn black.
  Do not implement multiple-light accumulation, shadows, or a backend global
  uniform buffer in this milestone. Acceptance: component API/serialization and
  scene copy/duplication round-trip; a focused example visibly changes a lit
  mesh on OpenGL, Vulkan, and D3D12; unlit materials remain unchanged;
  Debug/Release builds pass; architecture/API/example documentation is
  synchronized.
  Current implementation (2026-10-31): DirectionalLightComponent is retained
  with GameObject transform-derived direction, Color/Intensity data, scene
  copy/duplicate support and serialized color/intensity validation. The
  renderer selects the first active light, writes the three
  `GE_DIRECTIONAL_LIGHT_*` constants only when a material declares the full
  contract, and otherwise leaves that material unchanged; no active light uses
  the documented white default. SceneRenderHelpers adds a lit mesh shader and
  changes light direction/intensity at frame 45 while keeping its unlit image
  material unchanged. Serializer, component checks, example runtime and
  Architecture/API/example documentation cover the new contract.
  Verification: CMake and freshly regenerated Premake VS2022
  Debug/Release SceneRenderHelpers builds passed. Premake Debug/Release and
  CMake Debug/Release bounded 120-frame runs exited 0 and logged component
  checks, the frame-45 directional-light change and normal shutdown. Six
  CMake matrix runs passed separately for OpenGL, Vulkan and D3D12 in
  Debug/Release; Config.ini was restored to GraphicsAPI 3.
  Remaining: manual visible checks of the lit mesh and frame-45 transition,
  real window resize/minimize/restore, and long-run memory behavior are not
  verified. Multiple-light accumulation, shadows and a backend global uniform
  buffer remain intentionally out of scope.
  User explicitly requested commit/push without further compilation (2026-10-29).
  Publication scope includes the preceding native GAsset tools and editor import
  work, their regressions/docs, and this component consolidation. Exclude tmp,
  build outputs, local backend settings and dependency pointers. Review intended
  staged files, git diff --check, commit, push master and verify remote revision.
  Publication adds no further architecture or public API behavior.

- [ ] Wire or explicitly scope out 3D scene simulation. Current
  `Scene::OnAwake` creates `Physics3DWorld`, but `Scene::OnFixedUpdate` only
  calls `Physics2DWorld::Simulate`; no 3D step occurs through `Scene::OnUpdate`.
  Acceptance if supported: a scene-owned 3D rigid body advances under a fixed
  timestep in a focused example, with PhysX initialization and shutdown owned
  by the runtime. Otherwise document the 3D world as a manual/experimental API.
  This records a discovered limitation; no physics code change is in scope for
  the documentation audit.

- [~] Implement the game-world logic update contract (2026-10-31,
      user authorized).
  Scope: define one explicit scene tick pipeline that dispatches object and
  component lifecycle/update phases in a deterministic order, including awake,
  start, enable/disable, fixed update, variable update, late update, physics
  update/contact callbacks, and deferred destruction. `GameObject` must define
  its object-level behavior and forward the appropriate phases to retained
  components without making rendering or camera behavior depend on script
  execution. Design managed custom-script ownership and dispatch without
  resurrecting the removed legacy NativeScript component: decide whether script
  instances are components, object-attached behaviors, or a separate engine
  service, and define serialization, assembly selection, per-instance state,
  failure handling, and shutdown ownership. Keep application/renderer code
  backend-neutral and preserve standalone Mono runtime initialization.
  Acceptance: a focused example exercises active/inactive objects, component
  update order, fixed and variable updates, script construction/update/teardown,
  script exception/failure behavior, physics callback ordering where supported,
  pause/resume, and deferred destroy. Object/component/script state and scene
  serialization round-trip consistently; Debug/Release engine/example builds and
  available-backend runtime checks pass; architecture/API/example documentation
  describes the final tick contract and unsupported cases.
      Current implementation (2026-10-31): Scene now owns a deterministic
      awake/start/enable/fixed/update/late/pause/resume/destroy pipeline and
      a bounded fixed-update accumulator. GameObject stores script class names,
      forwards lifecycle phases to retained components, and tracks object-level
      awake/start state. Component adds OnEnable, OnDisable, OnFixedUpdate,
      OnPause and OnResume hooks. Scene serialization round-trips a Scripts
      sequence and rejects unknown keys. ScriptEngine owns managed
      ScriptBehaviour instances per scene/object/class, updates a minimal C#
      GameObject wrapper, dispatches lifecycle phases, stops future dispatch
      after a managed exception, still invokes OnDestroy for cleanup, and owns
      GC handles through removal/shutdown. The GameWorld example exercises
      active/inactive objects, lifecycle order, fixed/variable updates, pause,
      failure, disable/enable, scene script roundtrip and deferred destroy.
      Verification: CMake and Premake Debug/Release GEngine, GEngine-ScriptCore
      and GameWorld builds passed; GameWorld exited 0 in all four
      generator/configuration combinations with expected script records,
      logged managed failure, teardown and normal shutdown. Logs are
      build/gameworld-*.log.
      An isolated `GE_WITH_MONO=OFF` CMake Debug GEngine build also passed,
      confirming the native-only script stubs compile. No native-only script
      runtime is claimed: enabling scripts without Mono remains an explicit
      failure.
      Remaining: physics world stepping/contact callback ordering, managed
      Transform/engine API access, project assemblies beyond ScriptCore,
      editor script authoring/compilation, long-run behavior, and backend
      rendering interaction with script-driven scene changes are not complete.
      C++ is the native engine/application language but is not a generic
      scene-script language; only C# ScriptBehaviour classes are dispatched.
