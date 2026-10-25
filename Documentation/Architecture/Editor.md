# 旧编辑器移除记录

2026-10-25：用户授权移除旧 ImGui 编辑器。其源码、专用资源、Mono 副本、
GizmoController、旧启动器与构建目标已删除。历史实现可从 Git 历史查看。

当前编辑器使用 [Qt Host/Worker 架构](QtEditor.md)，启动与验证见
[编辑器 README](../../GEngine-Editor/README.md)。完整 Scene/Play/Qt Gizmo 仍待实施；
移除旧实现不代表这些功能已迁移。引擎和游戏继续使用 Dear ImGui，EditorCamera
作为引擎组件保留。第三方聚合仓库内的 ImGuizmo 源码指针暂保留，但已无构建使用方。
