# Scene and physics follow-ups

- [x] Component consolidation (2026-10-29, user authorized): only StaticMesh,
  Image and the existing Camera remain attachable. Move transform, UUID, name
  and active state to GameObject base data. StaticMesh owns a shared mesh asset
  and material; Image owns an image asset and material with actual sampled draws.
  Remove MeshFilter/MeshRenderer/ImageRenderer, script/physics/light components
  and scene-driven simulation/contact dispatch. Keep standalone runtime services
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
