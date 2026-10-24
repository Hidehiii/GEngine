# GEngine Editor

Editor-only Qt source: `../GEngine/GEngine-ThirdParty/qtbase`, tag v6.10.3,
commit `7ddbc87d8e14ce51d2957ea72d0a6077593d5ff4`. It contains Core/Gui/Widgets
but is not a prebuilt SDK. The source is managed in the shared dependency
repository; engine and game targets do not link Qt. Qt Host/build/deployment
integration remains planned, so current launch commands still use the old editor.

UI 选型已确定：新编辑器使用 **Qt 6 Widgets**，**Dear ImGui 用于游戏内 UI**。
Qt Host/EngineWorker 迁移尚未实现；下述启动方式仍运行现有 ImGui 编辑器。
现有 ImGuizmo 是过渡实现，Qt 视口工具需要独立迁移，不直接嵌入 ImGui 面板。
Qt 面板、图像显示、输入路由及部署验收见[编辑器 TODO](../TODO/EditorArchitecture.md)。
E4-G 另行跟踪：Qt 变换工具完成并验证后再移除 ImGuizmo 子模块及旧适配器；
当前尚未删除，游戏内 Dear ImGui 不受该移除计划影响。

ImGuizmo is now a pristine upstream submodule at
`105e4fde45e4e74aae8973d98750820ef7aca7e2` (v1.92.5 WIP header).
Only this executable compiles it, behind `UI/GizmoController`; initialize
submodules recursively before building. Check translate/rotate/scale, Ctrl
snapping, local/world modes, grid alignment and hover/capture after resizing.
These interactive checks are separate from bounded startup/shutdown runs.
Migration verification: CMake Debug/Release engine/editor builds and all six
120-frame backend runs passed. A headless ImGui check covered operation/space
drawing, translation dragging and snapping. Full Premake builds and remaining
real-window interactions are still unverified; see the dependency TODO.

架构新增硬性要求：引擎在独立 EngineWorker 进程运行，崩溃/挂起后 EditorHost
仍能编辑、保存并重启 Worker。当前编辑器仍为同进程实现，尚不具备此隔离能力。
详见架构的“故障隔离与恢复”与 TODO 的 E0。

新的[编辑器架构设计](../Documentation/Architecture/Editor.md)与
[接口契约](../Documentation/API/Editor.md)描述文档、命令、运行会话及视口边界。
状态为提案；[阶段与验收](../TODO/EditorArchitecture.md)尚未实施，不代表下述
编辑器已提供撤销、项目服务或完整材质预览能力。

Presentation uses backend attachment UV origins. Check that panel text and scene
objects are upright, and that moving an object upward agrees with its gizmo on
each backend; a normal bounded exit alone does not verify orientation.

Build `GEngine-Editor` with CMake or Premake and launch from this directory so
`Assets` and `Resources` resolve. Select GraphicsAPI in a local Config.ini:
1 = OpenGL, 2 = Vulkan, 3 = D3D12. Preserve your preferred backend selection.

The editor provides hierarchy/transform editing, scene save/load/duplicate,
play/pause/stop, content browsing, camera navigation and transform gizmos.
Scene/game previews currently show image colors and mesh vertex colors. Textured
and lit/custom material preview and camera MSAA are explicitly unsupported.
Managed scripting requires `Resources/Scripts/GEngine-ScriptCore.dll` and
`mono/lib/mono/4.5/mscorlib.dll`; native logic and physics remain available
without them. Missing managed runtime files no longer prevent startup.

Run `RunEditor.bat Debug` or `RunEditor.bat Release` from any directory. The
launcher finds a VS2022 CMake/Premake build and sets the correct working directory.

Set `GENGINE_EDITOR_FRAME_LIMIT=120` for a bounded startup/render/shutdown check.
Expected: editor panels, colored preview geometry, usable toolbar and no backend
validation errors. Check viewport replacement on resize, minimize/restore,
scene save/open, play/pause/stop, and gizmo translation/rotation/scaling manually.
Do not treat a build or normal exit as proof of visible pixels or interaction.
Verification progress is in the [editor TODO](../TODO/Build/ThirdPartyDependencies.md).

On 2026-10-24, CMake VS2022 Debug/Release builds and six backend runs reached
120 frames and exited normally without captured validation errors. Release
launch from this directory also passed. Pixel/interaction inspection was not
completed because computer-use app access timed out; manual checks remain open.
