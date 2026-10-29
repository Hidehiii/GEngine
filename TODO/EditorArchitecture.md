# 编辑器架构重设计

- [x] E1.9 资源导入（2026-10-29）：Project 菜单加入 Import resource，选择
  源文件及项目 Assets 内目标目录，调用纯 C++ Engine Tools 单资源导入。
  保留原始文件、拒绝覆盖已有资源，异步执行并显示失败原因，成功刷新/选择资源。
  支持现有 texture/mesh/font/shader/include 源格式；不新增解码器或 GPU API。
  验收：工具和 Host Debug/Release 构建；导入/重复/无项目/越界/失败回归；
  文档、示例使用说明同步。导入期间禁止项目切换及打包，关闭等待作业完成。
  完成：新增 import-resource 原生命令，复用 C++ Envelope，保留源文件，以同目录
  临时文件及禁止覆盖重命名发布；Host 异步执行、错误提示和 Assets 选择已接入。
  CMake 加入 ImportCheck/CTest；Premake Host glob 已覆盖新源文件，补充其工具
  构建依赖。修复本次首次构建发现的既有字体读取 QString -> QByteArray 编译错误。
  验证：独立 VS2022 开发环境下 Host/工具 Debug、Release 构建通过；两个配置
  EditorImport/EditorProjectSettings/EditorRecovery/EditorProtocol 各四项 CTest
  全通过。导入检查覆盖文本/二进制内容、源文件保留、选中目标、重复拒绝、无项目、
  目录越界、导入中项目切换阻止、无效 UTF-8 的后台失败及菜单状态恢复。
  Tests/GAssetTools.py 两配置空 PATH 验证原生转换/修复及单文件无覆盖导入通过。
  git diff --check 与文档链接/内容审查通过。Qt 部署保留缺少翻译 catalog 的提示。
  剩余：原生文件选择器人工操作/取消与导入期间关闭窗口未人工验收；Premake 未
  重新生成/构建。导入只封装已有格式，不收集外部依赖或验证解码，64 MiB 上限；
  无渲染/IPC 变更，不重复引擎后端运行。未提交，保留先前工具改动与 tmp/。

- [x] E1.8 所有示例项目描述（2026-10-28）：为 Example 下七个示例提供同名
  .gproject，保留 FrameGraphTriangle 现有设置；其余示例配置各自的空默认编辑器
  场景，避免将含尚不支持组件的历史场景设为默认。原生示例程序保持现有工作负载。
  验收全部描述格式/相对路径/场景存在，实际 Host 打开七个项目；同步示例说明、
  Architecture/API，无引擎接口或构建配置变更。
  完成：补齐 DeferredRender、GpuDriven、InputSystem、SceneRenderHelpers、
  ShaderCompile、Triangle 的描述及各自 Default.GEScene，新增 Example/README.md
  覆盖列表和空场景预期。七个描述的名称/schema/相对路径/文件存在检查通过；
  Release Host + D3D12 Worker 逐个通过启动默认场景及设置恢复自检，证据
  build/example-project-<name>.log。无代码/构建改动，未重建引擎或重跑各原生工作负载；
  OpenGL/Vulkan 未重验。保留上项未提交改动及 tmp/，本项未提交/推送。

- [x] E1.7 项目设置与默认场景（2026-10-28）：每个 .gproject 保存
  settings.defaultScene（项目根相对路径），Project > Project settings 可选择、
  原子保存；打开 .gproject（菜单/命令行）自动选择其默认场景，切换项目不沿用
  上一场景。兼容旧双字段文件；未设置显示空项目视口，缺失/越界路径明确诊断，
  不用引擎示例冒充默认场景。保留本机打包偏好；原生导出器接受新设置。
  验收：设置往返/取消/失败保存、路径边界、旧格式、项目切换、真实 Worker
  自动加载和启动参数，Debug/Release Host/工具构建与既有恢复回归。
  完成：Project settings 对话框选择/保存默认场景，兼容 schema 1 旧双字段文件；
  新建项目生成 Assets/Scenes/Default.GEScene 并设为默认。打开文件、切换/关闭
  项目、空/缺失设置会选择/清理对应 Worker 及图像；失败保存保留旧状态。
  原生工具接受可选 settings 对象，FrameGraphTriangle 附带默认编辑器场景。
  验证：CMake Host/原生工具 Debug/Release 构建、协议/恢复/新设置 CTest 全通过。
  ProjectSettingsCheck 覆盖对话框错误/Save/Cancel、真实失败保存、路径越界、
  旧格式/错误字段、缺失场景修复、新项目和切换/关闭。实际 D3D12 Worker 在
  两个配置均通过默认场景加载/替换；分别验证 --project 与首个 .gproject 参数
  的启动图像/路径。原生元数据回归两配置各 9 案例通过；Release 实际自动编译/
  打包/增量/取消/自定义 EXE 与图标/资源包运行回归通过（88 秒）。
  证据 build/project-settings-{debug,release}-build.log、
  build/project-settings-{Debug,Release}-tests.log、build/project-settings-engine-*.log、
  build/project-settings-metadata-*、build/project-settings-packaging-release.log；
  build/project-settings-dialog.png 已审阅。入口 Auto 继续使用新 CMake Host。
  本轮 GPU 接口/IPC 未变；未重复 OpenGL/Vulkan、Premake 和人工文件选择操作。
  完整 Scene 编辑、Play 和已打包游戏启动场景行为不在本项范围。未提交/推送。

- [x] E1.6 单输入自动打包（2026-10-26）：Project 打包只选输出目录；删除程序
  路径、独立打包设置/预览入口，`.gproject` 不含任何打包字段。自动构建项目
  src 与引擎并归档资源，取消终止自己启动的构建进程树，失败保留旧包。验证
  单字段对话框、项目纯元数据、真实构建/打包/运行与 Debug/Release Host。
  完成：schema 1 限定 name/schemaVersion，NewProject 生成最小 src/Game.cpp；
  Package project 仅有输出字段，参数/源码构建自动推导。启动器变为原生
  EntryPoint 资源包适配，输出根目录直接 EXE/.gpak/DLL，输出偏好留本机。
  Windows Job 包含打包/编译后代，取消及关闭不留下自启构建进程树。
  验证：CMake/Premake Debug/Release Host 四组构建与 PackagingCheck 通过，
  覆盖实际单字段对话框、自动真实 Release 引擎/游戏构建、归档、增量重复、
  编译失败保留旧输出、取消/重试、项目文件不变、本机偏好/关闭；原有诊断
  RecoveryCheck 四组通过。截图 work/output-only-packaging-dialog.png 已审阅，
  日志见本轮 Codex 工作目录 work/final-*-{packaging,recovery}.log。原生游戏
  三后端运行及恶意包检查见 S2.4/S5.1；未宣称 Debug 游戏/C# 或人工全手势验证。
  架构/API/指南/README 同步，未提交/推送。

- [x] E1.5 项目文件与编辑器打包（历史，已由 E1.6 替代，2026-10-26）：新增 `.gproject` schema 1，
  保存项目名称、原生游戏程序与脚本开关；打包输出只存本机设置，不写入项目文件。File 新建/打开
  项目文件，Project 菜单配置/预览/打包/取消，异步子进程不阻塞 Host。保留旧
  目录浏览和草稿/Worker；失败打开保留旧项目，失败保存不覆盖旧描述，打包
  失败保留旧发布，关闭等待自己启动的打包进程退出。部署导出脚本不依赖仓库
  绝对路径。验收项目往返/无效版本/保存失败、实际 GUI 动作与输出选择、异步
  预览/导出/重复零复制/错误恢复及 Debug/Release Host 构建与既有恢复检查。
  完成：ProjectFile/ProjectActions、项目新建/打开菜单与 --project 启动入口、
  包设置对话框/异步预览/导出/取消，示例 FrameGraphTriangle.gproject。按照用户
  更正，输出完全独立于项目文件：仅存 QSettings 本机偏好，通过 -Output 传给
  导出器；仅改输出不重写项目文件。项目改名/移动后可重新选择本机目的目录。
  验证：CMake/Premake Debug/Release Host 构建全部通过，四组 PackagingCheck
  与诊断 RecoveryCheck 通过；实际对话框 Save/动作、项目往返/版本/失败保存、
  本机输出持久化重开/项目无输出字段、异步/UI tick、目录排除、零复制、失败
  发布保持、取消/重试与关闭均覆盖。Release 对话框截图已审阅。证据在本轮
  Codex 工作目录 work/project-*.log、editor-project-build-*.log、
  project-premake-*.log 和 project-packaging-dialog.png。
  真实 Worker：已有 Premake Release Worker 与两个 CMake Host 配置的恢复检查
  通过；默认 CMake Worker 和 Premake Debug Worker 是未更新颜色 sink 的旧
  构建，其日志颜色回归失败。未为本项重建引擎/Worker，不宣称该旧构建通过。
  CMake/Premake 部署脚本与配套架构/API/README 同步；引擎与 IPC 不变。
  未人工遍历文件选择/鼠标操作、高 DPI 或长时目录/导出性能；未注册 OS
  文件关联，不实现 .pak/C#。未提交/推送。

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

- [x] 2026-10-24 提交并推送：发布已完成的编辑器兼容修复、视口方向修复及
  架构/故障隔离提案。验收：仅暂存对应源码/资源/文档，不包含本地配置、
  imgui.ini 或 tmp；检查暂存差异及格式，核对提交与远端分支一致。
  完成：`b977adde` 已包含对应源码、资源与 TODO/文档更新，并已在
  `origin/master` 上；本轮重新核对本地 `master` 与远端同步。

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
  - [x] E1.1（2026-10-25 继续实施授权）：项目目录上下文与只读资源工作区。
    Core 保存规范化根目录/generation，失败打开保留旧项目；Qt Model/View
    提供 Assets，Inspector 展示所选资源元数据，均可停靠/隐藏恢复。关闭/切换
    项目及资源删除清除选择，拒绝根外/符号链接逃逸；浏览不执行或修改资源。
    验收临时项目的打开/失败/切换/关闭、选择失效、六面板布局及原有草稿/Worker
    恢复回归，CMake/Premake Debug/Release 构建。完整 SceneDocument/格式、
    导入、组件编辑及资源打包仍待后续阶段，本轮不把目录浏览宣称为资产数据库。
    发现并修复：Qt 树/header 在模型替换时保留 selection model；私有视图显式
    回收未共享的旧/临时选择模型，重复切换保持有界。异步枚举会暂时清空索引，
    选择以存在的项目内路径为准，目录加载完成后按 generation 恢复高亮，避免
    将刷新误判为删除而丢失选择。
    完成：File 打开/关闭项目目录、Assets 树、只读资源 Inspector 与六面板布局；
    项目名显示于窗口/Assets，完整路径保留在提示和 Console。保留版本 2 布局，
    四面板旧状态可恢复。Core/Host 生命周期、架构/API/README 与构建说明同步。
    验证：CMake/Premake Debug/Release 构建及协议/诊断/真实引擎恢复均通过；
    Unicode 目录、真实树选择、失败打开保留状态、同名前缀兄弟目录越界、
    符号链接拒绝、文件/根删除、20 次切换及选择模型数量、故障后项目/草稿
    保留均通过。Windows Release 原生检查与截图复核通过；本轮真实引擎检查
    使用 D3D12，未重复 OpenGL/Vulkan 渲染矩阵（渲染接口与 Worker 未改）。
    证据：各构建目录 tests/recovery-tests/engine-tests.log，
    tmp/qt-project-workspace-Release.{log,png}。链接/差异检查通过。
    未验证：手动文件选择对话框、超大/网络目录和长时 watcher 内存；完整 E1
    场景文档/选择服务与 E2 命令仍开放。此轮实现未提交，文档提交 8a3526a5
    已单独推送至 origin/master；临时文件/本地备份未纳入。
  - [x] E1.2（2026-10-25 用户要求）：草稿文档多标签窗口。新建/打开新增标签，
    同一路径聚焦已有标签；每页独立文档、历史、保存路径、脏状态和文本控件，
    标签可重排/关闭，Ctrl+W 关闭当前页，退出逐页确认且 Cancel 保留所有页。
    保存失败或另存为冲突不修改文档/保存点；最后一页关闭后保留一页新草稿。
    验收多页编辑/撤销/保存/重排/关闭取消、重复打开、退出取消及 Worker 故障
    后各页保留，Debug/Release 与两种生成器检查。只扩展 .gedraft，场景多文档
    和多个引擎视口仍未实现；保留当前未提交的 E1.1 工作。
    完成：Documents 内独立页面模型/编辑器/路径/保存点；稳定页面身份支持
    重排，重复打开聚焦已打开页，路径冲突在写入前拒绝，最多 32 页。后台页
    关闭/取消恢复原活动页；全窗口关闭逐页确认，取消不移除其他页，已经完成的
    保存不回滚。页面删除/析构先断开回调，保存/撤销只作用于当前页。
    验证：CMake/Premake Debug/Release 构建、协议/项目/标签/诊断恢复与真实
    D3D12 Worker 恢复检查通过；原生 Windows Release 对话框与截图检查通过。
    覆盖独立历史、Undo/Redo 保存点、拖动重排 API、重复打开保留未保存内容、
    后台页关闭取消、Save/Discard/Cancel 退出分支、冲突不覆盖文件、最后一页
    替补和后台页崩溃保留。证据为各构建日志与
    tmp/qt-editor-document-tabs-Release.{log,png}；差异/文档链接检查通过。
    未人工逐项验证鼠标拖拽/快捷键/文件对话框，未做 32 页满载长时内存检查；
    标签会话恢复、逐页脱离窗口和 Scene 多文档仍未实施。此轮未提交或推送。
  - [x] E1.2 UI 修订（2026-10-25）：用户要求浏览器式主窗口顶部文档标签栏，
    不再把标签放在右侧 Documents 停靠面板内。保留独立文档、关闭确认与排序，
    顶部提供新建按钮、溢出滚动和完整文件名提示；工具面板仍共享并可停靠。
    验收顶部标签与内容同步、旧多文档回归、Debug/Release 构建及原生窗口检查。
    完成：BrowserHeader 固定置于菜单/工具栏上方，BrowserDocumentTabs 提供
    圆角活动标签、关闭/拖动/溢出滚动，+ 与 Ctrl+T/Ctrl+N 新建；内部标签隐藏。
    顶部与页面双向同步并阻断更新反馈；原有保存、历史、取消和 Worker 保护保留。
    CMake Release 使用独立 build/dist/browser-tabs 验证（原 Release 编辑器正
    由用户运行并锁住 EXE，未关闭/终止该窗口）；CMake Debug 与 Premake
    Debug/Release 均通过构建、协议/项目/多标签和 D3D12 引擎恢复检查。
    Windows 原生 Release 截图/回归通过，证据 tmp/qt-browser-tabs-Release.{log,png}
    与各构建目录日志。原用户进程保留；临时构建脚本未加入源码，不提交/推送。
- [x] E1.2 面板标签位置修正（2026-10-25）：用户截图指的是 Viewport、Hierarchy、
  Console、Assets、Documents 停靠组标签，而非草稿文档标签。所有停靠区域的
  面板标签应位于内容上方，包含恢复旧布局后的行为；保留分割、浮动与文档标签。
  验收：四个停靠区域均为 North，Debug/Release 构建与现有恢复回归通过。
  完成：MainWindow 设置全区域 North；Premake Debug/Release 构建、协议、
  布局/文档恢复及实际引擎恢复检查均通过（对应 qt-editor-logs）。本轮未单独
  人工拖拽复查，也未重建 CMake；渲染接口无变化。尚未提交/推送。
- [x] E1.2 浏览器面板主标签（2026-10-25）：按用户浏览器截图修正，窗口顶端
  显示 Viewport/Hierarchy/Console/Assets/Documents/Inspector 主标签，切换面板、
  重排、关闭隐藏、+ 重新打开。默认所有面板共享全窗口内容区；保留拖出/停靠。
  草稿文档标签移入 Documents，避免与主面板标签混淆。升级布局版本以应用新默认。
  验收：面板切换/隐藏重开/排序不丢文档和 Worker，Debug/Release 回归与截图检查。
  完成：BrowserPanelTabs 使用稳定 dock 名称导航，关闭仅隐藏、+ 重开；默认六面板
  tabify 为一个全窗口组，原生组标签隐藏并消除占位高度，保留 dock 标题拖动入口。
  Documents 内持有草稿标签，布局持久化版本 3，旧版自动回退到新默认。
  Premake Debug/Release 构建、协议、面板/多文档恢复及 D3D12 引擎恢复均通过；
  Windows 原生 Release 回归与截图通过，证据 tmp/qt-browser-panel-tabs-Release.{log,png}。
  CMake 本轮未重建；系统标题栏保留，主标签顺序未持久化，未人工遍历拖放手势。
  文档/API/README 同步，未提交或推送。
- [x] E1.2 停靠与启动入口修正（2026-10-25，2026-10-26 验证）：保留顶部浏览器导航，恢复可同时
  显示的默认分割布局及 Qt 原生停靠组拖拽入口，允许浮动、重新停靠、标签分组。
  RunQtEditor.bat 自动选取指定配置中最新的 CMake/Premake Host，支持显式指定
  生成器与只打印路径。验收：布局/浮动/分组回归及 Debug/Release 构建通过；
  启动选择检查涵盖新旧输出、缺失文件与参数错误；同步架构/API/README。
  完成：恢复左/中/右分割默认布局，原生组标签可见且位于组顶部，移除隐藏标签
  的事件过滤；顶部浏览器标签仅导航，不改变 dock 位置。布局版本升级到 4。
  RunQtEditor.bat 调用同目录 RunQtEditor.ps1，按 UTC 修改时间选择同配置最新
  Host；支持 Auto/CMake/Premake、-PrintPath，缺失显式生成器不静默回退。
  Premake Debug/Release 构建、协议、布局/浮动/重新停靠/分组与 D3D12 引擎
  恢复检查通过；Windows 原生 Release 图像审阅通过，证据
  tmp/qt-docking-launcher-Release.{log,png}。启动脚本在隔离假输出中检查两种
  时间顺序、显式覆盖、缺失回退/报错及非法参数，tmp/check-qt-launcher.ps1 通过。
  CMake 本轮未重建；未人工遍历鼠标拖放手势。未提交或推送。
- [x] E1.2 实际停靠标签样式替换（2026-10-26）：按两张截图，将原生停靠组
  标签直接改成浏览器式圆角/活动高亮/×/+；移除重复全局面板导航。保留 Qt
  自己管理的拖动、重排、分组、浮动与布局恢复，不替换为代理页面容器。
  + 菜单将所选面板加入当前组，× 仅隐藏面板。验收多组/重排后按钮对象正确、
  隐藏重开与分组恢复不丢文档/Worker、Debug/Release 构建和 Windows 截图检查。
  完成：移除全局 BrowserPanelTabs；BrowserDockTabs 直接装饰实际 Qt 停靠组，
  保留原生 tabData、拖拽、重排与布局；×/+ 控件随分组重排重新核对绑定，
  + 将面板加入当前组。固定唯一标题映射明确排除草稿栏，未使用 Qt 私有数据。
  Premake Debug/Release 构建、协议、停靠/多文档与 D3D12 引擎恢复通过；新增
  三面板分组重排后 × 目标、+ 重开到当前组、同名 Console 文档隔离检查通过。
  Windows 原生 Release 回归与截图审阅通过，证据 tmp/qt-native-browser-docks-Release.{log,png}。
  RunQtEditor.bat Release Auto -PrintPath 已选中最新 Premake Release；本轮
  无启动接口改动。CMake 未重建，未人工遍历拖拽/高 DPI/窄窗口溢出组合。
  Architecture/API/README 已同步；未提交或推送。
- [x] E1.2 停靠标签减薄（2026-10-26）：缩小实际停靠组标签的上下留白、
  圆角与 ×/+ 控件，使默认高度由约 40 降到约 26 逻辑像素；保留文字与按钮可用。
  验收 Debug/Release 构建、已有停靠恢复回归与 Windows 截图；同步文档。
  完成：上下 padding 8→3，控件 24→20，图标字号 20→16，圆角 10→6。
  Premake Debug/Release 构建、协议、停靠/文档/引擎恢复回归通过；Windows
  原生 Release 截图审阅通过，证据 tmp/qt-compact-dock-tabs-Release.{log,png}。
  文档与 README 同步；引擎 API 未变。CMake/额外 DPI 组合未重验，未提交/推送。
- [x] E1.3 Console 原色日志（2026-10-26）：保留引擎原日志颜色，Worker 在
  stderr 输出携带颜色的文本，Host 增量解析并用 Qt 文本格式显示，不能靠文本
  关键字猜测级别。保留纯文本复制、UTF-8/颜色跨管道分块、换行、300 行上限，
  单行和控制序列有界；进程退出/重启重置解析状态，stdout 二进制协议不变。
  验收颜色/Unicode/截断/重启回归、真实引擎绿色信息日志、Debug/Release 构建
  与 Windows 截图，架构/API/README 同步。
  完成：EngineWorker 的 ANSI stderr sink 按现有 Windows spdlog 级别配色输出；
  ConsoleOutput 增量解析 UTF-8/16 色 SGR，以 Qt 字符格式插入文字。Host 消息
  默认色，复制不含控制码，HTML 字面显示，长行/序列/历史有界，退出排空后重置。
  Premake Release 与 CMake Debug 构建、协议/停靠/文档/引擎恢复通过；回归覆盖
  按字节切分中文/SGR、红/绿/黄/红底颜色、重启复位、长行/序列与 300 块上限。
  实际 D3D12 Worker 信息日志绿色格式检查通过，原生 Windows Release 截图
  审阅通过：tmp/qt-colored-console-Release.{log,png}。OpenGL/Vulkan 本轮未运行，
  渲染 API 未变。未实现完整终端/扩展色/自定义系统调色板；未提交或推送。
- [x] E1.4 SelectionService 抽取（2026-10-26 跟进）：把资源选择状态从 Qt
  控件移入无 Qt 的 EditorCore 服务，使用项目 generation 和项目内相对资源
  ID 作为权威状态；Assets 面板只保留 UI 投影。验收：选择、同项目重验、
  切换/关闭项目与资源删除使选择失效；空 ID/空 generation 拒绝；协议测试
  增加无 GUI 回归，现有项目工作区与 Worker 恢复回归不回退。
  完成：Core/SelectionService 保存 generation+相对资源 ID，提供 Select/Validate/
  Clear/ClearScope；ProjectWorkspace 不再持有私有 m_Selected/m_SelectionGeneration，
  Qt 树与 Inspector 仅投影权威选择。选择删除、项目切换/关闭、空 ID/scope
  和无关 scope 不清除的行为进入 ProtocolTests。
  验证：CMake ProtocolOnly Debug 通过；重新构建 Debug Qt SDK 后，
  CMake 完整 Debug Host/EngineWorker 构建与协议/恢复/真实引擎检查通过，
  证据 build/qt-editor/windows-x86_64/Debug/{tests.log,engine-tests.log}。
  Premake Debug 的四个编辑器目标构建及协议/恢复/真实引擎检查也通过，证据
  build/premake-vs2022/windows-x86_64/Debug/qt-editor-logs/。
  Architecture/API/BuildSystem 文档、README 与本 TODO 已同步。Release
  本轮未重跑；对象选择、E2 命令事务和 Worker 文档同步仍开放。
- [ ] E2：引入 EditorCommand、CommandHistory 和输入路由；属性编辑及 Gizmo
  拖动形成事务。验收：创建/删除/复制/组件/变换可撤销重做，一次拖动一个记录，
  保存后撤销仍准确标脏；文本输入、弹窗、相机与 Gizmo 不抢快捷键。
- [ ] E3：拆出 PlaySession 与显式状态机。验收：运行副本不污染编辑场景，
  Play/Pause/Resume/Stop 重复循环资源回收正确，初始化失败回到 Edit，
  运行时对象选择不串入编辑文档；暂停单步只在引擎提供明确能力后启用。
- [x] E4.1 实时场景视口（2026-10-28）：用共享 SceneRenderer 和显式命令录制
  替换 Worker 私有三角形；连续拉取、按视口尺寸输出、64 KiB 分块且最多一帧
  在途，心跳独立。UTF-8 场景路径检查与读取保持一致，覆盖中文文件名。
  支持打开现有颜色 ImageRenderer 场景，未提供场景时展示明确
  标记的引擎示例 Scene。验收组件呈现兼容、连续变化像素、resize、三后端
  Debug/Release 构建与恢复检查；记录同步读回及未支持的场景能力，不宣称 E4 全部完成。
  完成：组件新增显式命令重载，保留呈现适配；SceneRenderer 录制活动对象的
  ImageRenderer/MeshRenderer，Worker 使用该接口绘制离屏 Scene。移除旧三角形
  shader；示例 Scene 有动态变换，File 菜单可打开颜色 .GEScene，未知组件/纹理
  明确报错。Host 使用单帧 credit、64 KiB 分块、尺寸/请求校验和隐藏暂停。
  验证：CMake native-only 引擎/Worker/SceneRenderHelpers 与 Qt Host Debug/Release
  构建通过。OpenGL/Vulkan/D3D12 各配置均通过 EditorSceneViewport.py（71 帧、
  40 次尺寸替换、中文路径、方向/像素、独立心跳与不支持组件失败）；D3D12
  显式开启 debug layer/info queue，无警告/错误。每配置三后端 Host 恢复检查
  通过，包含至少 12 帧、801×451 替换、隐藏暂停/恢复、打开/恢复示例 Scene、
  crash/hang/malformed/退出；协议/诊断恢复 CTest 通过。组件示例三后端每配置
  120 帧退出 0，交替共享录制/兼容绘制及材质/网格替换通过。
  证据：build/editor-live-{debug,release}*、build/editor-live-tests/、
  build/editor-live-host-{Debug,Release}-{1,2}.log；原生 Release 截图
  build/editor-live-native-release.png 已审阅。优化 Python (-O) D3D12 回归也通过。
  40 次替换采样：OpenGL handles 561→561；Vulkan Debug 816→816、Release 812→811；
  D3D12 Debug 1085→1087、Release 1504→1504。私有字节增长约 8–38 MiB，不能
  据此声称长期内存稳定；更长压力、高 DPI/真实最小化/人工拖动与 Premake 本轮未验。
  已部署 CMake Debug/Release，启动器 Auto 选择新 Release；保留各部署 Config.ini。
  完整材质序列化、相机/Gizmo/Play 和异步读回仍开放，E4 不标完成。未提交/推送。
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
