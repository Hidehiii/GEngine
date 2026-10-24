# Rendering architecture refactor

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
