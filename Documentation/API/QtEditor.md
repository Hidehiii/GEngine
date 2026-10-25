# Qt 编辑器模块与协议契约

C# 编译与 Worker 重启的目标边界见 [托管脚本拟议契约](ManagedScripting.md)。
当前 Host/Worker IPC 不变，Worker 仍禁用脚本；此文档更新未实现脚本消息。

状态：**E0.1 诊断与 E0.2 引擎预览契约已实现，其余完整场景契约为提案**。

## 已实现的 E0.1 契约

Host 的四个面板使用稳定 objectName：HierarchyDock、DocumentDock、ConsoleDock、
ViewportDock。QDockWidget 均设置 AllDockWidgetAreas 与 Closable/Movable/
Floatable；View 菜单复用 toggleViewAction，ResetLayoutAction 恢复默认分割。
saveState/restoreState 使用版本 2；面板关闭只是隐藏，重新打开保留文档及
诊断连接。无新增公共引擎接口或 IPC 消息。

头文件：`GEngine-Editor/Protocol/EditorProtocol.h`（标准 C++，无 Qt/引擎依赖）。
所有整数显式小端编码，禁止按 C++ struct 内存布局发送。

| 字节偏移 | 内容 |
| --- | --- |
| 0–3 | ASCII GEIP |
| 4–5 | uint16 version = 1 |
| 6–7 | uint16 Kind（1–11） |
| 8–15 | uint64 非零 session |
| 16–23 | uint64 request |
| 24–27 | uint32 payload length，最大 65536 |
| 28–31 | uint32 reserved，必须为零 |

Kind 依次为 Hello、Ready、Ping、Pong、Shutdown、Ack、DiagnosticFrame、Crash、
Hang、Malformed、EngineFrame。Hello payload 为 UTF-8 GEngine.EditorDiagnostic/1；
Ready 由诊断 Worker 返回同值，真实引擎 Worker 返回 GEngine.EditorEngine/1。
Host 按 Ready 标识限制接收 DiagnosticFrame 或 EngineFrame，禁止混用。
真实 Worker 在设备和管线初始化后才 Ready，Ping 同时触发下一帧预览。
EngineFrame 为 160×90 RGBA8，复用下面的尺寸/步长结构，仍受 64 KiB 上限约束。
真实 Worker stdout 保留为二进制协议，CRT stdout 日志重定向到 stderr；
Host Console 接收核心/应用日志。Stop/故障注入也适用于真实 Worker。
EngineRuntime/RenderGraph/Texture2DArray 公共签名不变，同步读回仅发生在子进程。
Worker 一次握手后拒绝会话不匹配、重复/倒序 request 或非法命令；其他控制
payload 为空，响应复用请求 ID。图像 request=0，payload 前三个 uint32 为
width/height/stride，后续为顶部起始 RGBA8；诊断 Worker 只生成 128×80 图案，
Host 拒绝大于 256×256、stride 不等于 width×4 或字节数不符的数据。

Decoder 支持分片/粘包，Feed 每次最大 64 KiB，缓冲上限为两份最大报文；
头部错误后保持失败，必须 Reset，EOF 的残包通过 HasPartial 报告。Host
按请求 ID 验证 Ready/Pong，旧会话丢弃；连接状态与超时见架构实施节。
诊断工具不接收场景数据，不宣称支持下面的 LoadSnapshot/ApplyEdits 等消息。

`Core/DraftDocument.h` 提供 Reset/Edit/Undo/Redo/MarkSaved/Text/Dirty 查询，
最多 256 个独立历史状态。`.gedraft` 仅含 format、version、notes 三个字段，
明确不是现有 `.GEScene`；不支持字段的文件被拒绝而非丢字段重存。
`Host/WorkerSupervisor` 提供 Start/Stop/Inject 与状态/日志/QImage 回调，仅在
Host 主线程使用；Inject 只发送给其启动的子进程。Host 源码不加载 GEngine。
ProtocolTests 和 Host --self-test 分别覆盖解析/历史与真实子进程恢复。
Host --self-test --engine-check --worker <absolute-exe> 进一步验证真实引擎标识、
至少两帧、三角形像素/方向及 Console 日志。CMake/Premake 的完整 -Test 包含
该检查；协议专用模式仍不要求 Qt 或 GPU。逐后端/配置证据见编辑器 TODO。

## 其余目标契约（待实施）
本页配套 [Qt 编辑器目标架构](../Architecture/QtEditor.md)，实施归
[编辑器 TODO](../../TODO/EditorArchitecture.md)。名称表示职责，不代表现有签名。
本次删除不改变通用引擎 public headers；旧编辑器 GizmoController 已移除。

## 模块契约

| 接口边界 | 输入与结果 | 所有权/错误规则 |
| --- | --- | --- |
| DocumentService | New/Open/Save/Close → Success/Cancelled/Error | Host 拥有文档；失败不替换旧文档或保存点，保存不依赖 Worker。 |
| DocumentQuery | DocumentId/Generation + ObjectId → 只读数据或 NotFound | 不返回跨帧裸组件引用，面板不持有 Scene/GameObject。 |
| CommandService | 类型化编辑意图 → Applied/Rejected | 验证与修改原子执行；菜单、delegate、快捷键共用命令。 |
| CommandHistory | Begin/Preview/Commit/Cancel、Undo/Redo | 一次拖动一个记录，取消恢复；同步 revision 与历史保存点独立。 |
| SelectionService | 文档或运行会话限定标识 | 对象删除、换文档或断连时清除相应选择。 |
| PlaySessionProxy | Play/Pause/Resume/Stop → 待决请求及最终状态 | 等 Worker ACK/状态事件后确认；不把进程启动当作游戏启动成功。 |
| WorkerSupervisor | Start/Restart/Shutdown、请求与异步事件 | 有截止时间，只拥有自己的 Worker；断连使待决请求失败。 |
| ViewportController | 视图、相机、内容矩形、DPI、输入 → 渲染/工具意图 | Qt UI 在主线程；工具通过命令修改文档，不向 Worker 借指针。 |

EditorCore/共享协议使用独立值类型；Qt adapters 转换 QString、QVariant、QImage
和 Qt events。引擎 adapters 转换 Scene、组件和后端数据。二者不能将自身
库类型放入共享 DTO。跨边界的数组/字符串由接收方拥有或在明确调用范围内借用。

## 消息与顺序

首版由 QProcess 异步管道承载。stdout 专用于协议，stderr 用于日志；不在
Qt UI 线程同步等待 Worker。头部采用显式字节序/字段编码，包含 magic、
protocol version、kind、payload length、session generation、request ID、
document ID/generation 与 revision。固定字段大小、数值范围和编码格式须在
E0 的协议头及测试向量中定义，禁止依赖 struct padding 或本机指针大小。

| 消息 | 前置条件与结果 |
| --- | --- |
| Hello / Ready / Error | 核对协议、build ID、backend capabilities 和容量；不兼容则不进入 Ready。 |
| LoadSnapshot | 完整文档及 revision；验证/建镜像成功才 ACK，失败不发布部分镜像。 |
| ApplyEdits | 携带 base/new revision；只按匹配顺序应用，缺失或冲突请求重同步。 |
| Play / Pause / Resume / Stop | 检查连接与运行状态；Play 指定已同步的文档 revision。 |
| ViewportRequest | ViewId、resize generation、相机 revision、矩阵、尺寸和输出要求。 |
| Input | 输入序号、视图/焦点 generation、位置/按键/文本及 ReleaseAll。 |
| FrameChunk / FrameReady | 有界二进制块与完整帧元数据；只发布全部校验通过的图像。 |
| Heartbeat / Shutdown | 健康状态与有界关闭；退出监测不能依赖 Worker 游戏循环仍在推进。 |

ACK、Ready、FrameReady 分别表示操作已应用、初始化完成、图像完成。重复
RequestId 不重复执行同一会话的变更；重启不重用 session generation。
过期/错误会话回复丢弃，超时返回明确错误；Host 文档不因镜像失败回滚。
控制命令队列有界，溢出时标记需完整重同步，不能静默丢弃文档修改。

## 图像与输入

Frame metadata 至少包含 ViewId、resize generation、FrameId、DocumentRevision、
CameraRevision、宽高、stride、format、色彩空间、alpha 约定和总字节数。
首版采用顶部起始 RGBA8；颜色转换与后端翻转由 Worker 负责，Host 不获取
GPU handle/texture ID，也不等待 Worker fence。

容量初始设计为每图像最大 64 MiB、每块最大 64 KiB、每视口最多两个完成帧，
握手可协商更小值。验证乘法溢出、行跨度、块范围与完整性后才创建 Host 图像；
失联半帧丢弃。QImage 背后存储必须覆盖绘制生命周期，不能指向循环覆写槽。
大图解析在 I/O/解析任务执行，UI 更新通知合并，控制消息优先于后续图像块。

Qt 输入按实际图像区域转换，明确逻辑/物理像素及 letterbox，不重复换算 DPI。
游戏文本提交与按键分开，失焦/关闭视口发 ReleaseAll，旧焦点输入丢弃。
Worker 先向游戏 ImGui 投递，再按 capture 状态投递游戏逻辑；Host 不创建
游戏 ImGui context。SceneViewport 的 Qt 工具输入不转发成游戏操作。

## 兼容与验收边界

现有 `.GEScene` 需经 Host 独立 codec 往返验证，未知字段/组件必须保留或
明确拒绝不安全保存；不将现有引擎 Serializer 直接链接到 Host。
Qt Gizmo 与命令接口通过 E4-G 验收后才移除现有 ImGuizmo 依赖。

协议字段、状态和资源上限在实现中必须有不依赖 GUI 的解析/顺序回归检查；
视口、输入、Play 与崩溃恢复还需真实 Qt 应用和逐后端运行验证。本文不将
源码依赖、旧编辑器运行结果或文档完成视为新协议已验证。
