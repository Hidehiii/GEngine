# Rendering architecture refactor

2026-10-29 component consolidation is tracked in SceneAndPhysics.md: only
StaticMeshComponent, ImageComponent and CameraComponent remain attachable.
GameObject owns transform/identity/active base data. ImageComponent samples its
image asset and material; StaticMeshComponent owns its mesh asset and material.
Scene physics/scripts/lights are removed; backend APIs and submission lifetime
are unchanged. CameraComponent keeps the existing camera behavior.

2026-10-31 component verification follow-up: ModelImporter manager ownership is
RAII-only on failed FBX loads, OpenGL maps separate Texture2D/sampler reflection
to combined image samplers, and Vulkan accepts separate samplers during material
update. Debug/Release engine, SceneRenderHelpers and EngineWorker builds plus
six 120-frame OpenGL/Vulkan/D3D12 runs passed. Manual visible, real resize and
long-run memory checks remain open in their focused TODOs.

2026-10-31 backend module split is tracked in [Phase 4](Rendering/Phase4.md):
CMake and Premake now build OpenGL, Vulkan, and D3D12 as named static modules
and link all three into GEngine. Runtime selection and public rendering APIs are
unchanged; generator/backend verification is recorded in Phase 4.

Editor E1.9 imports source files through the native C++ tool into project GAssets.
The Host remains independent of the engine. Rendering APIs, GPU ownership and
Worker IPC are unchanged; acceptance is tracked in TODO/EditorArchitecture.md.

2026-10-29 GAsset tooling integration is tracked in ScriptingAndPackaging:
conversion/reference repair are implemented directly in the GEngineTools C++ CLI.
Rendering APIs, resource formats, backend synchronization and ownership are unchanged.

2026-10-29 graph access-declaration synchronization is tracked in
[FrameGraphCore](Rendering/FrameGraphCore.md). It validates state/read-write
and state/stage compatibility at declaration time, validates stage against the
pass queue during compilation, and removes illegal Vulkan access-mask fallback.
No public signature changes; OpenGL/D3D12 ordering remains conservative.
CMake Debug/Release builds and the FrameGraphTriangle 120-frame matrix passed on
OpenGL, D3D12 (Debug Layer active), Vulkan shared, and Vulkan dedicated queues.
Vulkan validation layers were unavailable on this host.

2026-10-28 D3D12 cross-execution UAV synchronization repair is tracked in
[FrameGraphCore](Rendering/FrameGraphCore.md). It adds resource-owned submitted
fence/value/queue tracking for UAV buffers/images, retaining owners through each
submission; no common rendering signature changes, CPU device-wide waits or
OpenGL/Vulkan behavior changes are introduced. Release supports explicit debug-
layer validation for this regression, and discarded recordings publish no token.

2026-10-28 planned engine-specific resource data types are owned by
[ScriptingAndPackaging](ScriptingAndPackaging.md#engine-specific-resource-data-types-planned-2026-10-28).
The plan separates CPU/file schemas from backend GPU objects and requires
available-backend upload/lifetime verification. Current rendering APIs, native
ownership and submission semantics are unaffected; no implementation is authorized.

2026-10-27 configuration/cache correction: Config/Config.ini is external to .gpak;
Cache/Shaders/<API>/ persists directly under the game root. Literal shader include
contents participate in cache identity. Publication preserves these state folders.
GPU interfaces and submission lifetime are unchanged; evidence is tracked in
ScriptingAndPackaging.

E1.6/S2.4/S5.1 supersedes the editor's earlier manual deployment: metadata-only
.gproject, output/name/optional-logo UI, automatic native Release compilation and .gpak output.
EntryPoint validates/mounts resources through a Windows runtime adapter before
Application construction. Renderer/GPU interfaces and ownership are unchanged.
Fresh engine/game source compilation and three-backend archive runs are tracked
in ScriptingAndPackaging; the older S2.1–S2.3 notes below remain history.

Custom executable/logo follow-up (2026-10-27): correct ICO serialization and
temporary-file ownership, keep icon build inputs stable for incremental export,
and clear cached icon selection when the user removes it. GPU APIs, backend
synchronization and rendering ownership are unaffected; current verification
is recorded in ScriptingAndPackaging.

S5.2 direct archive reads (2026-10-27) replace asset extraction with owned-memory
read adapters for shader/includes, YAML, images/cubemaps, fonts and FBX. Vulkan
file-image decode now normalizes RGB inputs to supported RGBA8. Existing upload
calls copy/consume CPU bytes before returning; queue lifetime and GPU interfaces
are unchanged. Resource probes and per-backend verification belong to S5.2.

E1.5/S2.3 adds `.gproject` descriptors and asynchronous Host packaging actions.
Output paths stay in per-machine editor preferences or an explicit CLI argument,
never the project file. GPU interfaces, frame protocol and backend ownership are
unchanged; no .pak renderer/asset reader or scene-format conversion is introduced.

S2.1 adds an explicit-manifest directory exporter only. Engine rendering APIs,
backend synchronization and GPU resource ownership are unaffected; no game/backend
runtime validation is claimed by the deployment regressions. Full game export
and asset format integration remain in TODO/ScriptingAndPackaging.md.

S2.2 migrates deployment to PowerShell and adds a FrameGraphTriangle adapter and
package-local launcher with writable session state. C++/GPU interfaces remain
unchanged. Existing CMake Release binaries passed packaged OpenGL/Vulkan/D3D12
120-frame readbacks; this is deployment evidence, not a rebuild of current source.

Editor Console color preservation changes only worker log transport and Host text
formatting. GPU APIs, backend behavior and binary frame protocol are unchanged.

Compact dock tab height is a Host styling change; renderer APIs and backends are unaffected.

Editor styles actual native dock group tabs as browser tabs, without a duplicate
global navigation strip. Native docking remains; document tabs live within Documents. This Host-only layout adjustment
does not change rendering APIs, backends or worker ownership.

Editor E1.2 adds independent .gedraft document tabs and per-tab history/close
confirmation in the Qt Host. It does not create additional engine workers,
viewports or Scene documents, and changes no renderer/IPC interfaces.

Editor E1.1 adds read-only project/asset browsing and metadata Inspector in the
Qt Host. Rendering APIs and IPC are unchanged; selecting project assets does not
replace the independent GPU triangle with a Scene renderer. New UI checks are
tracked in TODO/EditorArchitecture.md, with full scene rendering still pending.

2026-10-25 legacy editor removal: the ImGui editor application, private panels/
GizmoController, assets and build entries are removed. Qt Host/Worker is retained;
engine/game ImGui and EditorCamera are unchanged. ImGuizmo has no build consumers,
but its aggregate source pin remains pending separate dependency cleanup.

E0.2 engine preview integration is tracked in TODO/EditorArchitecture.md:
GEngine worker renders a 160x90 offscreen triangle through existing RenderGraph,
copies to a texture-array layer and synchronously reads it inside the child at
roughly 2 FPS. Qt receives RGBA8 frames and stderr logs; no engine public rendering
API change. High-resolution asynchronous readback and Scene editing remain open.

E0.1 now builds an independent Qt Host and protocol diagnostic child. Its
128x80 CPU image is NOT an engine/backend render test. No engine rendering API
changed in E0.1; full SceneRenderer/asynchronous worker readback and Qt gizmos
remain open in E0/E4. E0.2 adds the limited real preview described above.

Canonical Qt editor target design: [architecture](../Documentation/Architecture/QtEditor.md)
and [contracts](../Documentation/API/QtEditor.md). Editor E0–E6/E4-G owns the
implementation; old Editor.md is history, not the new frame/IPC specification.

Qt dependency storage is now the shared aggregate's qtbase submodule (6.10.3),
with editor-only consumption planned. No engine/backend rendering APIs or build
links changed; Qt SDK and Host viewport integration remain in editor E0–E6.

Planned ImGuizmo removal is tracked in editor E4-G: first verify Qt Host gizmo
rendering/input and undo transactions, then remove the old adapter and submodule.
No rendering interfaces or runtime dependencies change with this plan entry.

UI decision: Qt 6 Widgets owns the planned EditorHost panels and CPU-image
viewport; Dear ImGui stays in the game/Worker rendering path. Qt gizmo overlays
and input forwarding require migration under [editor E0–E6](EditorArchitecture.md).
Current ImGui editor rendering remains a transitional implementation.

ImGuizmo migration: editor-owned GizmoController now owns frame/context/viewport
integration; no Gizmo export remains in the engine. Backend rendering interfaces
are unchanged. Pin and verification are tracked in the dependency TODO.

Editor E0 adds process isolation: engine viewports render in EngineWorker and
send completed CPU images to an independent EditorHost UI; no cross-process
GPU handles in the first milestone. See [scope and checks](EditorArchitecture.md).

Editor flip repair: final presentation now uses GUIUtils attachment UV endpoints;
scene/camera coordinates and public rendering interfaces are unchanged. Evidence
and remaining visual checks are tracked in the editor compatibility TODO.

## Topic index

- [Editor architecture proposal and E1–E6 acceptance](EditorArchitecture.md):
  E1.8 supplies a project descriptor/default editor scene for every Example;
  this data-only addition changes no rendering API or native example workload.
  E1.7 adds per-project default-scene settings and automatic Worker selection on
  project open; renderer APIs/GPU ownership/IPC are unaffected.
  E4.1 now tracks shared SceneRenderer recording and continuous, sized image
  transport (implemented; OpenGL/Vulkan/D3D12 Debug/Release pixel/resize and Host
  recovery checks passed). Synchronous readback remains bounded to the Worker;
  full camera/material serialization, async readback and long-run memory remain open.
  E4 owns the editor viewport/render bridge migration; shared scene rendering
  must preserve backend-neutral interfaces and explicit GPU lifetime.

- [Editor compatibility repair](Build/ThirdPartyDependencies.md#editor-compatibility-repair-2026-10-23-in-progress)

- [In-progress dependency upgrade](Build/ThirdPartyDependencies.md#in-progress-imgui-box2d-and-spirv-cross-upgrade-2026-10-22)

- [Planned engine Gizmo forwarding interface](Build/ThirdPartyDependencies.md#planned-engine-owned-gizmo-forwarding-interface)

- [Component and helper rendering gaps](Rendering/ComponentRenderHelpers.md)
- [Integration status](Rendering/Integration.md)
- [Combined milestones](Rendering/Milestones.md)
- [Architecture phases](Rendering/ArchitecturePhases.md)
- [Smoke-test example](Rendering/SmokeTest.md)
- [Frame graph phase](Rendering/FrameGraph.md)
- [Resource ownership](Rendering/ResourceOwnership.md)
- [Phase 4 and verification](Rendering/Phase4.md)
