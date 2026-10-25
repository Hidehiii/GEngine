# 编辑器架构重设计

## 设计文档（2026-10-24）

- [x] 新建统一的 Qt 编辑器架构文档：整合 Qt Host/EngineWorker 依赖、文档与
  事务、运行状态、协议/图像/输入、Qt Gizmo 替代、构建与 E0–E6 迁移验收。
  验收：新文档成为目标架构入口；旧方案保留历史但明确被替代；API、索引、
  README 和渲染跟踪同步，无同进程/ImGui 编辑器目标的矛盾表述。仅文档，
  不实施 Qt UI、引擎接口或文件格式迁移；内容/链接检查及 diff --check 通过。
  完成：Architecture/QtEditor.md 与 API/QtEditor.md 已创建并设为新方案入口，
  旧 Editor.md 标记为历史，索引/服务页面/README/渲染跟踪已同步。相对链接、
  当前/计划状态及设计依赖边界已复核；git diff --check 通过。仅文档，无需构建。
  实施和故障/交互验证仍归 E0–E6/E4-G，不随文档完成而勾选。
  2026-10-24 用户授权提交并推送统一架构、子进程/Pipe 契约和配套文档；
  发布不包含功能实现、本地配置、窗口布局或临时测试文件。

- [x] 添加编辑器专用 Qt 6 源码依赖（2026-10-24 用户授权）：将官方 qtbase
  以固定 Qt 6.10.3 提交的子模块放在 GEngine/GEngine-ThirdParty/qtbase，
  由聚合仓库统一管理，但不链接到 GEngine/游戏。验收：Core/Gui/Widgets
  源码及许可证实际可用，上游工作区无修改，子模块指针固定、文档/API 同步；
  明确源码就绪不等于 SDK 已构建或 Qt 编辑器已实现。
  完成：v6.10.3 固定为 7ddbc87d8e14ce51d2957ea72d0a6077593d5ff4，源码及
  上游许可证就绪，子模块工作区无修改，Git 对象连接检查通过。中断下载已迁至
  聚合仓库，主仓库无编辑器目录的额外子模块。CMake/Premake VS2022 默认生成
  通过，未改引擎/游戏链接配置；文档链接和两仓库 diff --check 通过。
  剩余：Qt SDK 构建、Host 集成/部署和实际 Qt UI 运行验证仍在 E0–E6；
  此次不涉及引擎编译代码，不重建引擎。聚合仓库提交 e0a1a35 已推送到
  origin/master，主仓库 gitlink 随文档更新到此固定提交。
  2026-10-24 用户已授权提交并推送：先发布聚合仓库的 Qt 源码子模块，再提交
  主仓库固定指针和文档；本地配置、窗口布局及临时文件不纳入提交。

- [x] Qt 6 / ImGui 分工修订（用户已选定）：Qt 6 Widgets 作为新 EditorHost
  的 UI，Dear ImGui 作为游戏进程内 UI。验收：更新架构、接口、构建计划及
  README，明确 Qt 主线程/进程管理、视口图像、输入和 Gizmo 归属；现有 ImGui
  编辑器标为过渡实现，不将文档选型当作 Qt 迁移完成。链接和格式检查通过。
  完成：架构/API、构建计划、索引、README 与渲染跟踪已同步，Qt 官方接口
  文档及相对链接已核对，git diff --check 通过。本次仅更新设计，无需重建。
  Qt SDK 核验、实际 Host/Worker/视口工具及游戏 UI 示例仍按 E0–E6 待实施。
  2026-10-24 用户授权提交并推送本次 Qt 选型及 E4-G 文档修订；发布范围仅为
  设计、TODO 和 README，本地配置、布局及临时文件保留，不包含功能实现。

- [ ] 2026-10-24 提交并推送：发布已完成的编辑器兼容修复、视口方向修复及
  架构/故障隔离提案。验收：仅暂存对应源码/资源/文档，不包含本地配置、
  imgui.ini 或 tmp；检查暂存差异及格式，核对提交与远端分支一致。

- [x] 编写基于当前源码的编辑器架构方案。本次范围仅限文档；不授权实现。
  验收：记录现状、依赖与所有权、文档编辑和撤销、运行状态机、视口与 GPU
  生命周期、输入路由、资源服务、迁移阶段与逐后端验证标准；同步架构/API
  索引、编辑器 README 和渲染 TODO，并检查链接和 git diff --check。
  进度：已检查 EditorLayer、Hierarchy、ContentBrowser、Scene 公共头、现有
  渲染与服务文档。实现和运行结果不能作为本次设计已实现的证据。
  完成：架构与接口契约文档、模块依赖图、E1–E6 实施边界及验收标准已写入；
  已同步索引、服务页面、README 和渲染 TODO。相对文件链接检查通过，Gizmo
  计划锚点已核对，git diff --check 通过。本次仅文档，无需引擎构建。
  剩余：下列实现阶段全部未开始，不把设计完成等同于运行功能完成。

## 分阶段实施与验收

- [x] 2026-10-25 用户授权提交并推送：发布 Qt Host/停靠面板、独立引擎预览与
  日志、必要的 OpenGL/Vulkan 修复及旧编辑器移除。验收：仅暂存对应源码、
  构建、资源删除和文档；排除 tmp、本地备份与生成输出；检查暂存差异及格式，
  提交后核对远端 master 与本地提交一致。构建/运行证据见下方 E0.1/E0.2。

- [x] 2026-10-25 移除旧编辑器（用户授权）：删除旧 ImGui 编辑器源码、专用资源、
  Mono 部署副本、旧启动器及项目生成入口；保留 Qt Host/Worker 与所需字体，
  引擎/游戏 ImGui 和仍使用的 EditorCamera 不受影响。本地配置、布局先备份再
  移出旧路径。移除旧文档入口的失效源码引用，保留 TODO 历史；验收两种生成器
  不再生成旧目标，新 Qt Debug/Release 构建及协议/引擎恢复检查通过，diff 检查通过。
  完成：移除旧源码/启动器/生成目标、专用资源与缓存、Mono/脚本副本、旧生成
  项目和输出；字体只保留 Qt 使用的 Regular/OFL。历史文档替换为移除说明，
  引用转到 Qt 文档。引擎/游戏 ImGui、EditorCamera 和新 Worker 均保留。
  原本修改过的 Config.ini、imgui.ini 与 VS 用户配置备份到
  tmp/legacy-editor-local-backup-20261025，再移除旧路径；其他 tmp 内容未清理。
  CMake/Premake Debug/Release 构建、协议/诊断恢复/真实引擎恢复全部通过，
  证据沿用各构建目录 tests.log、recovery-tests.log 与 engine-tests.log。
  ImGuizmo 源码/include 构建引用均已删除；共享聚合仓库的子模块指针未修改，
  依赖源码清理由 E4-G 剩余项跟踪。本次不提交或推送。

### E0.1：Qt Host 与子进程/Pipe 基础（2026-10-24 用户授权）

- [x] E0.2 真实引擎接入（2026-10-25 用户授权）：新增链接 GEngine 的独立
  EngineWorker，Qt Host 显示真实 GPU 离屏预览并接收引擎日志；诊断 Worker
  保留独立测试入口。首版使用有界低分辨率、请求驱动 CPU 读回，不新增引擎
  公共渲染接口，不冒充完整 Scene 编辑。验收：Debug/Release 构建、真实帧和
  日志到达 Host、停止/重启与崩溃后编辑器仍可操作；分别记录三后端结果。
  同步 CMake/Premake、启动与文档；明确同步读回、固定尺寸及尚未接入的交互。
  接入发现的修复：OpenGL framebuffer 包装纹理未初始化宽高/格式，导致合法
  Texture2DToArrayLayer 被拒绝；初始化包装元数据，并以真实预览读回回归验证。
  Vulkan 同样缺少 framebuffer 包装尺寸，且颜色附件缺 TRANSFER_SRC usage；
  显式保留 borrowed 所有权、补齐尺寸和复制用途，验证正常停止/重复启动无双重释放。
  Debug Vulkan 验证进一步定位：无深度附件仍传入空 depth reference，并将深度
  access 配给 color stage（VUID 03077/02900/03088）；修正条件引用与颜色写入
  access，通过同一无深度预览检查回归，不以 Release 空白帧充当成功。
  完成：正常启动自动连接真实 Worker，显示 160×90 GPU 三角形并转发核心/应用
  日志；约 2 FPS 请求驱动读回、顶部起始 RGBA8、重启保留草稿。Application
  兼容入口满足 Shader 缓存单例依赖；Host 不链接引擎。部署保留已有 Worker 配置。
  验证：CMake 和 Premake Debug/Release 构建、协议、诊断恢复、真实引擎恢复
  全通过。CMake 六组 OpenGL/Vulkan/D3D12 × Debug/Release 分别通过两帧、
  三角形颜色/方向、Console、崩溃/挂起/坏包/停止/重启与草稿保存检查；六份
  日志未发现 GPU 错误，Vulkan Debug 无验证层错误。Windows 原生 Release
  检查与截图复核通过；证据 tmp/qt-engine-*-verified.log、
  tmp/qt-engine-native-Release.{log,png}，构建目录中的 engine-tests.log。
  D3D12 Release 连续 101 帧/约 50 秒读回并正常退出；工作集从 124.67 MiB
  预热到约 127.94 MiB，50/75/100 帧为 127.97/127.93/127.94 MiB，未见逐帧
  增长趋势（短时 CPU 工作集，不代表长期 GPU 显存测量）。证据
  tmp/qt-engine-memory-check.log；本轮启动的构建和子进程均已结束。
  剩余：高分辨率异步读回/流控、Scene/输入/Play/Gizmo、GPU 显存及长时三后端
  测量仍由 E0/E4 跟踪；首次接通预览不等于完整编辑器迁移完成。

- [x] 2026-10-25 全面板停靠：将固定中央视口改为 QDockWidget；所有工作面板
  支持停靠、浮动、关闭/重新打开、分割和标签化。增加 View 菜单与恢复默认布局，
  保留版本化布局保存。验收：四个面板浮动/重停靠/隐藏恢复及布局往返，不影响
  草稿和 Worker；Debug/Release 构建及 Qt 恢复检查，文档/API 同步。
  已实现四个面板、View 开关、Reset layout 和版本 2 布局保存；工具栏也可移动/浮动。
  CMake Debug/Release 的 EditorProtocol、EditorRecovery 均通过；恢复检查覆盖
  浮动/重停靠/关闭重开/标签化/布局往返及草稿、子进程故障恢复。
  Windows Release 原生窗口检查通过，默认布局截图见
  tmp/qt-editor-docking-Release.png，日志见同名 .log；尚未人工逐项拖拽验收。
  Premake Debug/Release 构建、协议和恢复检查同样通过，证据位于
  build/premake-vs2022/windows-x86_64/{Debug,Release}/qt-editor-logs/。

- [x] 本轮有界里程碑：构建编辑器专用 Qt SDK，新增独立 Qt Host 和 Worker
  协议基础；Host 提供 dock 面板、独立可保存文档与子进程启动/停止/重启，
  有界版本化 Pipe、心跳/超时、日志和故障注入。验收 Debug/Release 构建、
  协议解析回归及子进程崩溃/挂起/重启后 Host 保留编辑/保存能力。
  初期 Worker 可明确标记为协议诊断模式；真实 Scene/三后端图像接入不得用
  诊断图案冒充。E0 全部验收（真实引擎渲染/故障矩阵）未完成时保持开放。
  同步 CMake/Premake 入口、启动说明、架构/API、剩余工作；保留旧编辑器和
  用户配置，不自动提交或推送。
  已实现：独立 Qt Host、标准 C++ draft/history/32 字节分帧协议、诊断 Worker、
  心跳/截止时间、CPU 诊断图像、原生崩溃/挂起/坏包注入、所属进程回收。
  .gedraft 为实验性笔记而非 .GEScene；现有编辑器/引擎 API 不变。
  验证：Qt SDK Debug/Release 已由固定源码构建并安装。CMake/Ninja 和
  Premake/MSBuild 两配置均构建，并通过协议/历史与 Qt 恢复检查；原生 Windows
  Release 检查也通过，子进程访问冲突退出码为 0xC0000005，Host 保持运行、
  保存/撤销/重启正常。导入表确认 Host 无 GEngine/ImGui，诊断子进程无 Qt。
  窗口图像检查后复用现有 Noto 字体及 OFL 并调整默认 dock 尺寸。
  证据：build/qt-editor/windows-x86_64/{Debug,Release}/tests.log、
  build/premake-vs2022/windows-x86_64/{Debug,Release}/qt-editor-logs/*tests.log、
  tmp/qt-editor-native-Release.log 和 tmp/qt-editor-native-Release.png。
  根 CMake Qt 选项已生成/构建验证；默认 CMake/Premake 生成无需 Qt。
  剩余：真实 EngineWorker/Scene、三后端读回/故障矩阵、大帧线程/流控、正式
  文档 codec、Qt Gizmo 与交互/长时间资源测量。上述 E0 全验收仍开放。
  未将 Qt 内部 OpenGL 支持或 CPU 图案当作引擎后端验证；现有子模块保持干净。

- [x] 2026-10-24 设计修订：将“引擎崩溃不带崩编辑器”纳入架构，替换原先
  同进程目标，补充 Host/Worker 所有权、异步协议、图像传输、故障恢复与限制。
  同步 API/README/渲染跟踪；内容及链接复核，git diff --check 通过。
  仅设计完成；E0 实现与故障注入尚未开始。

- [ ] E0（新增前置阶段）：实现 EditorHost / EngineWorker 进程隔离。编辑器
  文档、历史和 UI 不加载 GEngine DLL；场景预览、Play、脚本/物理、引擎资源
  加载在 Worker 内运行。验收：启动失败、访问冲突、abort、强制退出、挂起、
  IPC 断开均不结束或阻塞编辑器；未保存修改与 Undo 保留，可保存并重启 Worker。
  明确版本化 IPC、超时、会话 generation、视口 CPU 图像传输和关闭回收契约；
  OpenGL/Vulkan/D3D12 Debug/Release 分别执行故障注入。GPU 驱动/系统故障
  不承诺仅靠进程隔离完全恢复。E1–E6 均须遵循此边界。
  Qt 前置验收：新增独立 Qt 6 Widgets Host（Core/Gui/Widgets），使用
  QMainWindow/QDockWidget、QProcess 和异步消息；不加载 GEngine/ImGui。
  CMake/Premake 各有目标及 Qt 包路径/部署说明；游戏/引擎独立构建不要求 Qt。
  先完成 CPU 图像 QImage/QPainter 显示，不直接嵌入 Worker 原生窗口。
  统一方案采用 QProcess 异步标准管道的版本化分帧协议，控制优先、有界二进制
  图像分块及完整快照重同步；解析容量、旧会话/重复请求和半帧错误纳入验收。

- [ ] E1：拆出 EditorSession、SceneDocument、SelectionService、PanelHost 和
  ProjectContext，先保持单文档与既有文件格式。验收：新建/打开/保存/取消、
  失败不覆盖旧文档、脏状态提示、选择在关闭/删除后失效；现有面板可用。
  面板迁移为 Qt Widgets：Hierarchy/Assets 使用 Model/View，Inspector 使用
  类型化编辑控件；Qt model/delegate 的修改仍必须走命令，不绕过文档。
- [ ] E2：引入 EditorCommand、CommandHistory 和输入路由；属性编辑及 Gizmo
  拖动形成事务。验收：创建/删除/复制/组件/变换可撤销重做，一次拖动一个记录，
  保存后撤销仍准确标脏；文本输入、弹窗、相机与 Gizmo 不抢快捷键。
- [ ] E3：拆出 PlaySession 与显式状态机。验收：运行副本不污染编辑场景，
  Play/Pause/Resume/Stop 重复循环资源回收正确，初始化失败回到 Edit，
  运行时对象选择不串入编辑文档；暂停单步只在引擎提供明确能力后启用。
- [ ] E4：抽出 ViewportController、EditorRenderBridge 和 ViewportPanel，随后
  接入共享 SceneRenderer。验收：不保留两套材质语义；离屏目标/最终 UI
  依赖明确，正向画面、DPI、缩放、最小化、替换和关闭可验证；OpenGL、
  Vulkan、D3D12 Debug/Release 分别记录像素、验证输出及资源增长情况。
  共享渲染能力不足时保留显式标记的现有颜色预览；不把它标成正式场景渲染。
  Qt SceneViewport 使用 Host 的绘制/命中测试 Gizmo 覆盖层，返回变换命令；
  不在 Host 引入隐藏 ImGui context，也不把工具绘制交给会崩溃的 Worker。
  2026-10-25 更新：旧调用已随旧编辑器删除，ImGuizmo 子模块暂保留且无构建使用方。

### E4-G：Qt 视口工具替代并移除 ImGuizmo（计划）

- [ ] 在 Qt SceneViewport 实现网格、移动/旋转/缩放手柄、鼠标命中测试、
  拖动捕获与取消；支持 Local/World、位移/角度/缩放吸附及透视/正交相机。
  工具绘制与交互保留在 Host，Worker 失联时仍可编辑并保存。
- [ ] 将交互接入 E2 命令事务：一次拖动只生成一次撤销记录；Escape、失焦、
  删除目标或切换文档取消交互并恢复原值。验收 Undo/Redo、DPI、letterbox、
  resize、遮挡/选中状态和相机输入不冲突，记录三个后端的视口对齐结果。
- [ ] 替代工具通过上述验收后，删除旧 ImGuizmo GizmoController/调用、CMake/
  Premake 源文件与 include 路径、Dependencies.lua 条目，再从第三方聚合仓库
  移除 ImGuizmo 子模块及对应 .gitmodules 条目，更新清单/文档与主仓库 gitlink。
  保留已发布提交作为历史；不为移除 ImGuizmo 删除游戏内 Dear ImGui。
  验收：源码/生成项目无残留引用，CMake/Premake 构建与 Qt 编辑器交互验证通过，
  游戏 ImGui 仍可正常运行。发布时先提交/推送聚合仓库，再更新主仓库指针。
  2026-10-25 进度：用户已单独授权移除旧编辑器，旧调用、两种生成器与
  Dependencies.lua 条目已删除并通过构建；Qt 工具与聚合子模块清理仍未完成。

登记状态（2026-10-24）：仅记录替代/移除范围和验收；未实施，子模块与当前
编辑器代码保持不变。架构/API/README 已同步，链接与格式检查通过，无需重建。

### 后续阶段

- [ ] E5：建立项目路径、AssetService、文件操作结果和诊断面板。验收：项目
  切换不依赖进程工作目录；失败/取消不破坏文档；异步导入的过期结果被丢弃，
  GPU 创建遵循设备线程约束，退出时任务可取消并完成回收。
- [ ] E6：收敛兼容层与构建依赖。验收：Qt Host 启动替代旧 EditorLayer，面板
  不直接提交 GPU 工作、不持有裸 Scene 指针、不访问 EnTT 私有注册表；
  CMake/Premake 均构建，示例与手动验证矩阵记录完成/阻塞项后再删除旧路径。
  单独提供 ImGui 游戏内 UI 示例，验证 GameViewport 焦点、文本输入、DPI 与
  OpenGL/Vulkan/D3D12 的呈现/退出；不再使用 ImGui 构建编辑器主面板。

配套设计：[Qt 目标架构](../Documentation/Architecture/QtEditor.md)、
[Qt 接口契约](../Documentation/API/QtEditor.md)；旧 Editor.md 保留过渡记录。
Gizmo 已子模块化的当前实现由
[依赖 TODO](Build/ThirdPartyDependencies.md#planned-engine-owned-gizmo-forwarding-interface)
管理，不在此重复创建独立实现。
