# SceneRenderHelpers

This example uses the retained ImageComponent, StaticMeshComponent and CameraComponent components. Open
SceneRenderHelpers.gproject for an empty editor scene; the native application
creates its own test scene. GameObject owns its transform/name/active base data.

Set GraphicsAPI in Config.ini to 1 (OpenGL), 2 (Vulkan) or 3 (D3D12), then
launch with this directory as the working directory. Expected output: a red
checker quad on the left and cyan indexed triangle on the right. At frame 30
the quad turns gold through material replacement; at 60 the triangle grows;
at 90 the image asset reloads. SceneRenderer and GameObject::OnRender alternate.
The application disables scripting, physics, OpenSSL and ImGui.

Startup checks exercise two distinct 64-bit UUIDs, base data, CameraComponent settings,
cycle rejection, copy/duplicate ownership, parent remapping/deletion, and
persistent mesh/image/material shader/name scene roundtrips. An invalid mesh
index, malformed FBX and removed scene-component key must be rejected without
changing existing resources/scenes. Temporary fixtures are removed. GraphicsTool
point/line methods remain unsupported. Shader materials must match the
[component contract](../../Documentation/API/SceneAndPhysics.md).

Set GENGINE_SCENE_HELPER_FRAME_LIMIT=120 for normal bounded shutdown. On D3D12,
GENGINE_SCENE_HELPER_SYNTHETIC_RESIZE=1 requests 960x540 and 1280x720 at frames
40/80. This exercises target replacement, not OS resize/minimize/restore.
OpenGL/Vulkan ignore synthetic resize; Vulkan's extent must match its surface.

Manually check checker pixels/geometry, repeated window resize, minimize/restore
and normal close. Failure indicators: startup exceptions, missing checker/triangle,
corruption after replacement, validation errors or missing 120-frame exit log.
Review each backend's logs and long-run memory separately. A successful bounded
run confirms recording/checks/shutdown, not manual visibility or leak freedom.
Current build/runtime evidence is in [TODO](../../TODO/SceneAndPhysics.md).

Only mesh/image .gasset assets and material shader/name are persisted; arbitrary
runtime material overrides and procedural resources are not serialized. Shipped
historical physics scenes elsewhere now contain static base data and supported
components only; no scene physics or native-script component remains.
