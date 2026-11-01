# Rendering architecture refactor

2026-11-01 shader-container parser rewrite is implemented under
[ShaderGraph](Rendering/ShaderGraph.md): a CPU-only scanner/parser validates full
structure/state/stages, preserves exact Program bytes and reports source/line/
column errors. Shader loading commits parse data transactionally before disk
cache/compilation; cache fingerprints include parser revision 2. DepthWrite On
is corrected. CPU Debug/Release/corpus checks pass; new opt-in pipeline malformed-
shader rollback/cache checks remain runtime-unverified because this host's D3D12
version blocks complete engine/example builds. GPU/backend/public Shader::Create
semantics are unchanged; no visual graph or native scheduling work is included.

Version-exclusion recovery tips distinguish Windows-serviced SDKLayers, compiler
SDK retargeting and separate Agility integration; see
[BuildAndRun](../Documentation/Guides/BuildAndRun.md#changing-sdkruntime-versions).
This changes build diagnostic guidance only, not runtime selection/rendering APIs.

## D3D12 build-host version exclusion (2026-11-01)

User correction: the SDK pin/deployment approach below is superseded history. Current policy rejects Windows project builds when installed x64 system SDKLayers is exactly 10.0.26100.4768, with a console error and warning popup. CMake/Premake share the check; normal runtime creation/public APIs are unchanged. No pinned SDK or additional deployment dependency is required. See [D3D12Memory](Integration/D3D12Memory.md) for current verification and limits.

2026-11-01 D3D12 Event-handle growth has native API attribution under
[D3D12Memory](Integration/D3D12Memory.md): the system SDK layer retains Events
for pending cross-queue Wait calls, including after idle/device release. The
same probe is flat on Agility 1.619.5. The repair pins/prepares/deploys this
paired runtime and uses a backend-owned SDK 619 independent device factory,
with full synchronized queue validation. No queue ordering or public API
changes. Debug/Release 30,000-frame native Event ranges are 290..298 / 292..313;
the unfixed 6,000-frame regression fails with growth 21,149. Matched Release
layer/replacement controls, all-backend MSAA, D3D12 P1-P3 and true package/cold
launch pass with clean native output. P3 Debug stress keeps handles 748->748
across 6,000 frames. Exact logs, bootstrap/deployment negatives and remaining
OS/driver/build limits are in the owning TODO. The historical extra driver/heap
groups are not all attributed; the reproduced native Event leak is fixed.

2026-11-01 bounded P3 ordered HDR post-processing is implemented and verified,
under [RenderPipeline](Rendering/RenderPipeline.md). Payload v3 declares ordered
exposure/Reinhard settings over RGBA16F intermediates and an explicit linear/sRGB
RGBA8 output. A reusable graphics pass supports live enable/EV/order without
new GPU targets. RGBA16F has native three-backend mappings/queries and graph/asset
validation; existing enum values are preserved. The example composes two HDR
cameras before processing and checks reference pixels, rollback and resource
counts. Debug/Release four-backend 120-frame runs passed 18 invalid candidates,
four invalid live snapshots, two live markers, six replacements and 11 reference
probes each; Debug 6000-frame runs passed 349 replacements/403 probes per variant.
P1/P2/default/MSAA regressions passed all four variants in both builds with clean
native diagnostics. Logs and bounded memory: build/post-process, P3 TODO. Manual
display, packaged runtime, extended chains and HDR MSAA limits remain explicit.

2026-11-01 P2 camera stacks are implemented and programmatically verified under
[RenderPipeline](Rendering/RenderPipeline.md). Payload v2 adds stable camera IDs,
validated base/overlay trees, viewport/clear/load/target rules; a reusable stack
pass records serial per-camera graph work. v1 assets remain supported. Native
commands now support immutable compatible pass overrides and top-left viewport/
scissor state; Vulkan attachment layout metadata is initialized honestly. The
example checks real camera projections, depth, UI, disabled parents and switching.
Debug/Release four-backend 120-frame runs passed 16 invalid candidates, three
binding failures, two live activity changes, seven replacements and 11 paired
raw/sampled probes each; Debug 6000-frame runs passed 399 replacements/403 probes.
P1, original C++ and MSAA regressions passed both builds/all four variants with
clean native diagnostics. Logs: build/camera-stacks/final-*; limitations and
bounded memory observations are in the P2 TODO.
No scene-camera discovery, editor authoring or P3 effects are included.

2026-11-01 P1 asset selection is complete under
[RenderPipeline](Rendering/RenderPipeline.md). Versioned GAsset payloads,
runtime-owned pass validators/factories, target/dependency capability preflight,
Prepare and transactional frame-boundary selection are implemented. Debug/Release
four-backend 120-frame runs passed 18 failed candidates, seven switches and nine
pixel probes each; Debug 6000-frame runs passed 399 switches and 401 probes per
variant. Native diagnostics were clean and real HLSL compile failure recovered.
Vulkan command recording now retains framebuffer owners across replacement.
The opt-in RenderPipeline example switches two actual assets and verifies GPU
output/rollback/lifetime; existing C++ mode remains. P2 is described above;
P3 is described above.
Final logs: build/pipeline-assets/final-{Debug,Release} and final-stress-Debug.
Legacy shader-container parser hardening and broader archive/editor integration
remain separate; this milestone adds no camera stack or effects.

2026-11-01 per-format queries and MSAA are tracked in
[Phase 4](Rendering/Phase4.md#per-format-capabilities-and-msaa-2026-11-01).
Device queries expose attachment/sample/resolve support and format/framebuffer
dimensions; graph/native factories validate before allocation. Backend MSAA
color creation/views/resolves are repaired, and returned attachment references
retain their framebuffer through later queue submissions. FrameGraphTriangle
has opt-in repeated MRT/depth/fractional-edge readbacks and a D3D12 single-mip
array regression. CMake GE_WITH_MONO=OFF Debug/Release and all four 120-frame
backend variants passed three 24-case matrices; Debug 6000-frame runs also
passed. D3D12 debug-layer handle growth remains separately unresolved. D3D12 depth
resolve and Vulkan preserved multisample LOAD were unsupported in that baseline.
The authorized follow-up now implements queried native min/max D3D12 depth
resolve with sampled views and Vulkan stored MSAA attachments/LOAD dependencies.
DepthResolveMode is forwarded through graph declarations and render-pass cache
keys. Debug/Release four-backend 120-frame matrices passed 1536 color cases,
1184 numeric depth probes and 432 min/max edge checks, with actual-frame LOAD
and recreation. Debug 6000-frame runs passed separately; Vulkan handles stayed
near-flat, while the existing D3D12 debug-layer growth remains open. Native
diagnostics were clean. Final logs: build/msaa-depth/verified-{Debug,Release}
and stress-Debug; scope and untested hardware/build variants remain in Phase 4.

2026-10-31 foundation closeout is tracked in
[Phase 4](Rendering/Phase4.md#foundation-closeout-milestone-2026-10-31): consistent
portable/legacy attachment validation, allocation-free target-limit preflight,
backend color attachment limits and byte-correct D3D12 uniform capacity are
implemented. Debug/Release FrameGraphTriangle builds and all four backend
variants passed 120-frame regressions. Matched D3D12 lifetime controls completed;
allocation ownership, general queue ownership, per-format/sample capability
reporting and pooling/aliasing remain open. Historical foundation summaries now
point to current topic evidence.

2026-10-31 planned RenderPipeline extensions are tracked as P1-P3 in
[RenderPipeline](Rendering/RenderPipeline.md#planned-pipeline-extensions-2026-10-31):
pipeline assets/runtime switching, base/overlay camera stacking and ordered
post-processing (initially exposure/tone mapping). Each entry defines scope,
prerequisites and backend-specific acceptance. This is planning only; current
rendering APIs, backend synchronization and GPU ownership are unchanged.

2026-10-31 pipeline lifecycle follow-up is tracked in
[RenderPipeline](Rendering/RenderPipeline.md): centralized pass configuration,
paired frame phases and explicit graph rebuilds are implemented. Isolated
GE_WITH_MONO=OFF CMake Debug/Release builds and OpenGL/Vulkan shared/D3D12/
Vulkan dedicated 120-frame runs passed; Debug also passed 6,000 frames per
variant. Actual window/visible checks and long-run memory remain unverified.
Scene TODO
reconciliation preserves native script registration and standalone physics
ownership; no rendering backend API change is introduced by that audit.

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

2026-10-31 subresource scheduling is tracked in
[FrameGraphCore](Rendering/FrameGraphCore.md): `GraphicsCapabilities::
IndependentSubresourceScheduling` gates disjoint texture-range dependency
analysis. D3D12 and Vulkan can omit false inferred dependencies; OpenGL and
resources without metadata remain conservative. This is not parallel command
execution and does not enable pooling or aliasing.

2026-10-31 fine-grained pipeline stages are tracked in
[FrameGraphCore](Rendering/FrameGraphCore.md): GraphicsPipelineStage now
supports vertex, fragment, color-output, and depth-attachment declarations.
Vulkan maps them to native stage masks, D3D12 narrows shader-read resource
states, and OpenGL conservatively synchronizes the whole resource. This does
not imply parallel command execution.

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
- [Render pipeline framework](Rendering/RenderPipeline.md)
