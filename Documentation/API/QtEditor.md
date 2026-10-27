# Qt 编辑器模块与协议契约

C# 编译与 Worker 重启的目标边界见 [托管脚本拟议契约](ManagedScripting.md)。
当前 Host/Worker IPC 不变，Worker 仍禁用脚本；此文档更新未实现脚本消息。

状态：**E0.1/E0.2 诊断与引擎预览、E1.1 项目浏览、E1.2 草稿多标签、E1.4 选择服务已实现，其余场景契约为提案**。

## 已实现的 E0.1 契约

### E1.5 项目文件与导出操作

`.gproject` schema 1 示例：

```json
{
  "schemaVersion": 1,
  "name": "MyGame"
}
```

项目文件不含任何打包信息，schema 1 只接受 name/schemaVersion 两个字段。
`ProjectFile::Load/Save` 限制 1 MiB、校验版本/字段，Save 使用 QSaveFile；Root
为文件父目录，src/Assets/Config.ini/Licenses 均按项目根约定查找。
MainWindow::NewProject/OpenProject/CloseProject 管理此描述与只读 Assets 工作区；
无效打开不覆盖旧状态，新建不覆盖既有文件。目录浏览兼容入口不提供打包能力。
命令行 `--project <file.gproject>` 或首个 `.gproject` 参数打开项目，未注册系统关联。

MainWindow::SavePackaging(output) 仅保存本机输出偏好，绝不改写项目文件。
输出须与项目输入目录不重叠。PackageProject() 异步自动编译/归档/发布；PackagingRunning/
LastPackagingSucceeded 查询状态；CancelPackaging 终止该作业。QSettings 输出
键为 `packagingOutputs/<canonical-project-path SHA256>`，不进版本控制。

Project 菜单仅有 PackageProjectAction 与作业取消 CancelPackagingAction。
打包对话框对象名 PackagingSettingsDialog，唯一输入控件为 PackageOutput。
缺少 GEngineTools.exe、编译工具、引擎 SDK、
输入错误、保存失败、子进程非零退出均为可恢复失败，Console 提供细节。
作业期间不允许替换项目，草稿和预览仍可操作；成功/失败均恢复菜单状态。
导出命令为 `GEngineTools.exe export-project --project <file.gproject> --output <directory>`。
自动 Release 编译后，目标根目录仅包含入口 EXE 与 Runtime/、Data/、Metadata/
分类文件夹。资源不散放在输出根目录；原生入口负责包校验/展开，玩家不需要
PowerShell。格式/运行限制见
[打包指南](../Guides/GameExport.md)。不提供 Mono 或资源格式转换；图形 API/IPC 不变。

ConsoleOutput::AppendBytes(QByteArray) 接收原始 stderr 分块，跨调用保留 UTF-8
与 ANSI SGR 状态；支持 0/1/22/30–37/39/40–47/49/90–97/100–107。
AppendMessage(QString) 插入默认色 Host 文本，ResetStream() 结束未完成行并丢弃
未完成编码/控制序列及颜色状态，不清空历史。UTF-8、SGR 可以跨任意管道分块；
末尾不完整编码/序列在流重置时丢弃，CR 忽略、LF 保留，HTML 不解释。
WorkerSupervisor 新增 OnConsoleOutput(QByteArray) 与 OnConsoleReset() 回调；
OnLog(QString) 继续用于监督器诊断。退出排空 stderr 后、启动前发出 reset，
MainWindow 析构先断开回调。Stdout 包格式与引擎公共 API 均未改变。
Console 最多保留 300 文本块、单行 16 Ki UTF-16 单元，控制参数至多 64 字符。
长行截断但继续扫描换行/复位，未知终端控制不执行；不支持终端光标操作和扩展色。

Host 的六个面板使用稳定 objectName：HierarchyDock、DocumentDock、ConsoleDock、
ViewportDock、AssetsDock、InspectorDock。QDockWidget 均设置 AllDockWidgetAreas 与 Closable/Movable/
Floatable；View 菜单复用 toggleViewAction，ResetLayoutAction 恢复多面板分割布局。
saveState/restoreState 使用版本 4；旧版本布局回退到新默认。面板关闭只是隐藏，重新打开保留文档及
诊断连接。无新增公共引擎接口或 IPC 消息。

## 已实现的 E1.2 文档多标签

原生停靠组 QTabBar 标记为 BrowserDockTabs，直接使用浏览器式外观；已移除
BrowserPanelTabs 全局导航。CloseDockTabButton 隐藏对应 dock，AddDockTabButton
位于末标签按钮区，其 + 菜单将指定面板加入当前组、显示并激活（可能移动其他组的面板）。
Qt 继续管理 tabData、标签数量、选择、排序及拖拽。Host 不修改内部 tabData，
通过六个固定唯一标题匹配 dock，刷新中核对控件的 DockName/HasAdd 属性后更新按钮。
草稿内外标签栏显式排除，避免 Console 等同名文档被误认为面板。
重复面板标题/动态标题尚不支持；布局版本保持 4，分割/分组/浮动恢复契约不变。
公共文档 API 与 Worker 协议未改变。
停靠组标签采用上下 3 逻辑像素留白及 20×20 的 ×/+ 控件（默认约 26 高），
此为 Host 样式调整，文档 API、停靠生命周期与布局版本均不变。

MainWindow::NewDraft() 新增并激活草稿页，返回是否成功（最多 32 页）；
DraftCount() 查询页数，Document() 返回当前页文档引用，该引用在对应页关闭后
失效。SetDraftText/SaveDraft/SaveAs/Undo/Redo 操作当前页。QTabWidget 的稳定
对象名为 DraftTabs，其内部标签栏隐藏。可见标签栏为 Documents 内的
BrowserDocumentTabs（QTabBar），位于 Documents 内，提供重排/关闭/省略提示/溢出滚动。
NewDocumentTabButton（+）、Ctrl+T 与 Ctrl+N 新建，Ctrl+W 关闭当前页。
文档栏与页面容器双向同步索引/顺序，更新显示时阻断反馈信号，文档身份仍为页面。

OpenDraft 成功解析文件后新增页；若规范化路径已打开则仅聚焦已有页，保留
未保存内容。打开失败不新增页、不改变活动页；SaveDraft 拒绝另一个打开页
已占用的路径，写入仍用 QSaveFile。每页独立路径、历史、保存点和脏标记。
重排不移动文档所有权，非当前脏页也会让窗口标题显示未保存标记。

CloseDraft(index) 对指定页执行保存确认，失败/取消返回 false，不删除文档；
关闭后台页或取消后台页的关闭会恢复原活动页。
最后一页关闭后创建干净草稿。关闭窗口确认全部页面，取消时不关闭任何页或
Worker，已执行的成功保存保留。隐藏 Documents 面板不关闭文档；窗口布局
不保存文档内容或自动重开列表。本轮没有新增引擎/IPC/场景格式契约。

## 已实现的 E1.1 项目目录契约

`Core/ProjectContext.h` 提供 Open(path)、Close()、Root()、Generation() 与
Contains(path)。Open 使用 canonical 路径并要求目录存在；失败保留旧根与
generation，成功/关闭推进非零 generation。Contains 对存在的目标执行
canonical 与逐路径分量边界检查，不以字符串前缀代替目录归属。

`Core/SelectionService.h` 提供 Select(scope, id)、Validate(scope, exists)、
Clear()、ClearScope(scope) 与 Id/Scope/Empty 查询。资源选择的 scope 是
ProjectContext generation，id 是项目内相对路径；空 scope 或空 id 被拒绝。
Validate 遇到 scope 不匹配或资源不存在时立即清空选择。该类型只使用标准
C++ 数据类型，不依赖 Qt、引擎或文件系统枚举。

`Host/ProjectWorkspace` 提供 Open(QString)、Close()、Select(QString)、
ValidateSelection() 与根/选择/generation 查询，以及 Inspector/日志回调。
Select 拒绝根外、直接符号链接与缺失目标，失败清除旧选择；项目打开失败则
保留当前项目与选择。250 ms 校验删除/失联时使选择失效，项目根丢失时关闭
项目。ProjectWorkspace 把 SelectionService 的相对 ID 映射回项目内路径，
Qt 树控件只是选择状态与 Inspector 元数据的投影。只读 QFileSystemModel
隐藏符号链接，QTreeView 禁用编辑、拖放和执行。

MainWindow::OpenProject/CloseProject 对接 File > Open project folder / Close
project；AssetsTree 的当前选择更新 ResourceInspector 元数据。Inspector 为
只读资源信息而非组件编辑器。草稿/历史与 Worker 进程不随项目切换改变；没有
隐式打开 .GEScene、加载 Mono 或资产导入。该目录浏览入口不执行打包；上面的
`.gproject` 入口提供显式打包操作。本机输出偏好与项目文件分开保存。

下面的协议契约与 E0.2 相同，不随项目浏览变化；引擎公共接口保持不变。

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
