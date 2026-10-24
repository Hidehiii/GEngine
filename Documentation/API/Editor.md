# Editor 接口契约设计

## 已选定：Qt 6 编辑器 / Dear ImGui 游戏 UI（待实现）

新 Host 使用 Qt 6 Widgets，协议与文档核心不依赖 Qt 控件或引擎头。
Qt model/delegate 和 QAction 调用同一 CommandService；不额外维护一套
QUndoStack 历史。QProcess 退出/错误和异步消息驱动 WorkerSupervisor，Qt UI
线程不阻塞等待 Worker。QWidget 视口消费完成 QImage；图像存储在绘制期间
必须有效，更新使用排队通知。Qt 对象/QImage 不直接作为 IPC payload 传输。

游戏 UI 的 ImGui context 在 Worker/游戏内，捕获状态也在该端处理；Host
只发送聚焦 GameViewport 的逻辑输入与文本提交、释放/失焦消息。DPI、图像
letterbox 与像素坐标转换必须明确；失联时拒绝继续发送游戏输入。
Qt SceneViewport 工具走独立命中测试和编辑事务，不借用 ImGuizmo 类型。
下面已实现的 GizmoController 仅属于旧 ImGui 编辑器，不能直接用于 Qt 控件。
计划在 Qt 变换工具通过命中测试、吸附、取消及撤销验收后删除该适配器和
ImGuizmo 依赖；[E4-G TODO](../../TODO/EditorArchitecture.md#e4-gqt-视口工具替代并移除-imguizmo计划)
记录验收与清理范围。本次不改变任何接口，游戏内 Dear ImGui 保留。

## 已实现：编辑器私有 GizmoController

头文件：`GEngine-Editor/src/UI/GizmoController.h`，不属于引擎导出 API。
`BeginFrame()` 在编辑器 OnImGuiRender 开始调用一次，要求 ImGui 已 NewFrame。
`SetViewport(Vector2 position, Vector2 size, bool orthographic)` 在目标面板内
调用，使用 UI 逻辑坐标和当前 draw list。`Manipulate(view, projection,
operation, space, transform, snap)` 使用 Matrix4x4、GizmoOperation（None/
Translate/Rotate/Scale）与 GizmoSpace（Local/World），原位更新变换并返回
GizmoResult 的 Changed/Hovered/Active。snap 为可选 Vector3 指针，仅本次
调用借用；旋转单位为度，位移/缩放使用各轴数值。`DrawGrid(view, projection,
transform, size)` 使用同一视口。无 ImGuizmo/ImDrawList 类型泄漏或 DLL 导出宏。
以下其他模块仍为提案；命令事务尚未接入。

状态：**除上节 GizmoController 外均为提案，无新增引擎导出 API**。后续模块名称与操作表达职责，不是现有
函数签名；落地时以公共头为准。本页与[架构设计](../Architecture/Editor.md)
配套，所有实现工作由[编辑器 TODO](../../TODO/EditorArchitecture.md) 跟踪。

## EditorCore 的调用边界

硬性约束：EditorHost 不加载 GEngine；EngineWorker 承担预览、Play 和引擎
资源操作。以下“场景视图”在 Host 指独立文档 DTO，非引擎 Scene 引用。
跨进程操作全部异步，名称不代表直接 C++ 函数调用。

| 提议接口 | 输入/输出契约 | 约束 |
| --- | --- | --- |
| SceneDocument 查询 | 只读场景视图、文档 ID/generation、修订/保存点 | 借用视图仅当前调用有效，不给面板持久裸 Scene 指针。 |
| SelectionService | 文档/运行会话限定的对象 ID 或资源引用 | 每次解析验证 generation 与存在性；关闭/删除立即失效。 |
| CommandService | 类型化编辑意图；成功、失败原因或不可用状态 | 统一用于菜单、面板、快捷键；模型修改在安全主线程边界执行。 |
| CommandHistory | Undo/Redo、Begin/Commit/Cancel 事务 | 一次拖动一个记录；取消恢复原值；无效命令不进入历史。 |
| DocumentService | New/Open/Save/Close；成功、取消、失败 | 失败保留旧场景/路径/保存点；统一处理未保存确认。 |
| PlaySession | Play/Pause/Resume/Stop 请求与可用性 | 延迟到安全边界，失败回到 Edit；运行变化不写回编辑文档。 |
| PanelContext | 查询、命令与选择的窄服务集合 | 不作为任意服务定位器；面板无原生后端或私有注册表访问。 |

编辑命令的前后值必须可拥有、可验证，不能捕获跨帧的组件引用。删除与组件
移除的撤销需要可恢复快照；不支持的组件明确阻止该操作。保存状态基于历史
保存点与外部修改状态，不能简单在每次 Save 点击后清除 dirty。

## 渲染和工具契约

ViewportRequest（提案）描述视图 ID、场景 generation、相机、物理像素尺寸、
所需输出和工具覆盖层选项。EditorRenderBridge 返回逻辑视图输出及能力/错误
状态；Qt UI 消费 CPU 图像，不获取 VkImage、D3D 描述符或 GL 名称。
Worker 内 ImGui texture ID 不跨进程/跨帧缓存。尺寸为零时返回挂起状态，不创建无效目标。

场景数据在渲染提取时借用，异步/提交后仅保留独立快照和 GPU 资源强引用。
共享 SceneRenderer 及目标无关组件渲染契约是 E4 的前置工作，当前尚不可直接
按此调用。UI 逻辑尺寸、物理尺寸和后端 UV 各有明确边界，不通过修改相机
投影来修复最终 UI 翻转。

Gizmo 的[迁移记录](../../TODO/Build/ThirdPartyDependencies.md#planned-engine-owned-gizmo-forwarding-interface)
已改为编辑器私有适配器；后续将其变换结果接入命令事务。不要把 ImGuizmo
枚举、ImDrawList 指针或其导出宏放进新编辑器核心接口。

## 引擎 API 与兼容性

### Worker 协议（计划）

握手交换 protocol version、build ID、backend capabilities。消息包含 session
generation、request ID、document revision 和有界 payload；支持 LoadSnapshot、
ApplyEdits、Play/Pause/Resume/Stop、ViewportRequest、ACK、Error、FrameReady、
Heartbeat、Shutdown。请求有截止时间，断连使未完成请求失败，旧会话回复直接丢弃。
不传 Scene/GameObject 指针、ImGui ID、原生 GPU handle 或可跨进程解引用的地址。

FrameReady 描述已完成 CPU 图像：尺寸、格式、stride、色彩/UV 方向、frame ID
和文档 revision；有界槽与完成发布约定防止读取半帧。Host 只消费已完成帧，
不等待 Worker GPU；Worker 崩溃后仍可绘制失联提示、编辑/保存与重启。
运行 Inspector 读取带会话标识的快照，属性修改发请求，不能借用 Worker 内存。

WorkerSupervisor 返回 Starting/Ready/Unresponsive/Disconnected/Stopped 及退出
诊断。重启重发完整文档，不恢复失效的运行对象引用；Host 历史与保存点保持。
首版采用 CPU 图像传输，跨进程共享 GPU 资源为未承诺的后续优化。

当前 Application/Layer、Scene、GUIUtils、RenderDevice 与文件格式保持原样。
设计不承诺 Scene 已有稳定 ID 查找、完整 Stop、步进、通用组件快照、项目数据库
或共享离屏 SceneRenderer；这些能力在 E1–E6 中按需核实和增补。
新通用引擎头放在 `GEngine/include/GEngine`，实现放在对应 `GEngine/src/GEngine`；
编辑器专属头保留在编辑器目录，不进入 `<GEngine.h>`。

第一阶段保留单文档、现有 `.GEScene` 和旧面板适配器。任何接口迁移同时更新
调用方、Example、CMake/Premake 和后端能力说明；不以空实现维持编译通过。
