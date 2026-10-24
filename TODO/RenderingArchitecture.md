# Rendering architecture refactor

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
