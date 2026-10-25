# Rendering architecture refactor

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
