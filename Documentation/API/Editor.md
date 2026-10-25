# 旧编辑器接口移除记录

2026-10-25：旧 GEngine-Editor 应用及其私有 GizmoController 接口已移除，
不再提供旧编辑器目标、RunEditor.bat 或旧面板头文件。历史代码见 Git 历史。

当前接口见 [Qt 编辑器契约](QtEditor.md) 与
[Qt 编辑器架构](../Architecture/QtEditor.md)。引擎公共 API、游戏 ImGui 与
EditorCamera 不受本次删除影响；Scene 编辑、Play 与 Qt 变换工具仍在 TODO 中。
