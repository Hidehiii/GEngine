# SceneRenderHelpers

Open SceneRenderHelpers.gproject to open its empty default editor scene. The
native application below creates its own image and mesh objects in C++; those
objects are not serialized into the default .gscene. See the [example index](../README.md).

This example alternates `SceneRenderer::Render(*scene, presentationCommand)`
with `GameObject::OnRender` on successive frames, exercising both shared scene
recording and presentation compatibility with the same materials. The Qt Worker
uses that shared recorder in an offscreen render graph graphics pass.
This example also exercises the retained
`ImageRenderer` and `MeshRenderer` component paths. Set `GraphicsAPI` in
`Config.ini` to 1 (OpenGL), 2 (Vulkan), or 3 (D3D12), then launch the example
with this directory as its working directory so its shaders resolve.

Expected visible result: a red quad on the left and a cyan indexed triangle
on the right. After 30 frames the quad turns gold by replacing its material;
after 60 frames the triangle grows taller by replacing its mesh. Rendering
should continue without flicker, corruption, or validation errors. The
example uses no scripting, physics, OpenSSL, or ImGui.

Set `GENGINE_SCENE_HELPER_FRAME_LIMIT=120` for a bounded normal exit. Set
`GENGINE_SCENE_HELPER_SYNTHETIC_RESIZE=1` on D3D12 to send 960x540 and
1280x720 nonzero resize events after frames 40 and 80. This D3D12-only check
exercises `ResizeBuffers` but does not resize the native window or verify real
minimize/restore. Vulkan requires a matching native surface extent; synthetic
events without changing the window can stall acquisition, so they are ignored
on Vulkan and OpenGL. For the manual check, resize the window repeatedly, minimize
and restore it, and then close normally. Watch backend debug/validation output
and repeated-run memory behavior separately on OpenGL, Vulkan, and D3D12.

Startup checks that `GraphicsTool` point/line calls and textured image drawing
throw `std::logic_error`. A failure in those negative tests throws before the
first frame. `ImageRenderer` is color-only for now, and mesh materials must
match the full `Vertex` input layout. Shader compilation, the 30/60-frame
replacement logs, and the 120-frame exit log are noninteractive checks; they
do not prove that the colors or geometry were visible. The application and
backend logs are written to `Logs/GEngine.log` unless local saving was disabled
when the engine was built.
