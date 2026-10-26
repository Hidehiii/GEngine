# C#、Mono 与游戏打包

- [x] 2026-10-27 用户授权提交并推送：发布本次项目文件、编辑器单输出路径
  自动编译、原生 .gpak 与相关工具/回归/文档。范围包含前置目录导出原型历史，
  不包含 tmp、生成的工程/构建产物、本机输出或配置；子模块指针不变。
  已复核本轮构建/运行证据与工作区差异，提交前检查暂存清单及 diff --check；
  推送后核对本地 HEAD 与 origin/master 一致。

- [x] S2.4/S5.1 自动编译与原生资源包（2026-10-26 用户更正）：`.gproject`
  仅存名称/版本，移除整个 packaging 对象及 EXE 输入；项目约定 src/Assets。
  编辑器仅保留输出路径这一打包输入，自动 Release 编译引擎/游戏，生成 EXE、
  运行 DLL 和 .gpak 资源归档，发布目录无原始 Assets。原生入口校验/展开资源包
  到独立会话目录再启动，不要求玩家安装 PowerShell。首次为未压缩、SHA-256
  校验的版本化包，非加密。验收真实自动编译、三后端读回、包损坏/路径穿越、
  构建失败旧输出保护与单输入 UI；不把缺失编译产物当作可复制的已有 EXE。
  完成：schema 1 只含 name/schemaVersion；编辑器只有输出路径输入，移除
  EXE/设置/预览。BuildProject 自动初始化 VS 环境、CMake/Ninja Release 构建
  当前引擎和 src 项目；WriteResourcePack 生成 .gpak；PublishProject 直接发布
  EXE/.gpak/DLL，保留旧目录备份。源码不硬链接进输出，不覆盖用户新增/修改
  文件；重复运行仍调用编译器但可增量，未变输出 0 复制。Windows Job 负责
  编译子进程树，取消/退出一起回收。失败切换尝试回滚，不宣称目录替换原子。
  原生 EntryPoint 必须读取同名 .gpak，校验后展开到独立临时会话；无需玩家
  安装 PowerShell。路径、版本、条目/总长度和 SHA-256 校验覆盖；无压缩、
  加密或直接随机访问。默认 Config.ini 完整生成，Assets/Cache 排除。
  验证：从当前源码完整编译引擎与 FrameGraphTriangle（275 个构建步骤，
  GE_WITH_MONO=OFF）；原生 EXE 在 OpenGL/Vulkan/D3D12 各 120 帧、三项 GPU
  读回、退出 0/空 stderr 通过，无源码 cwd/开发 SDK PATH/脚本启动器。Unicode
  资源、缺 Config 默认配置、缺包、坏版本、损坏哈希、大小写重名、穿越与截断
  均覆盖。TestProjectPublishing 通过单文件增量/零复制/时间戳、备份和用户
  文件/损坏输出保护。四组 CMake/Premake Debug/Release Host 构建、真实单字段
  GUI 自动编译/归档/发布、错误/取消/重试及原有恢复均通过；截图已审阅。
  证据：本轮 Codex 工作目录 work/final-*-packaging.log、final-*-recovery.log、
  final-resource-pack-test.log、final-native-package-build.log，及构建缓存的
  configure/build.log；可审阅产物在 outputs/FrameGraphTrianglePacked。
  当前自动模式为原生 C++，GE_WITH_MONO=OFF 显式拒绝脚本初始化；标准构建
  仍默认 Mono，Premake 提供 --without-mono。Mono SDK 下载在本机 TLS 握手
  失败，未使用旧未验证副本替代。C#、通用场景依赖重写、压缩/流式读取、
  稳定资产身份、干净机器和长期 GPU 内存验证仍开放；Native Release 通过
  不代表 Debug 游戏或完整托管游戏已验证。文档/API/示例同步；未提交/推送。

- [x] S2.3 项目驱动导出与 Qt 操作入口（历史，已由 S2.4 替代，2026-10-26）：读取 `.gproject` 的
  原生程序，输出路径由 Host 本机设置或独立 CLI -Output 指定，绝不写入项目文件；收集程序目录 DLL、项目 Assets（排除 Cache）、Config.ini、
  Licenses 与启动器。Host 提供文件选择/设置保存和异步预览/导出/取消，复用
  ExportGame.ps1，CMake/Premake 部署脚本。验收真实编辑器项目测试、隔离导出
  与失败/重试；资产仍为原始目录文件，不实现 .pak、自动编译、C# 或场景解析。
  完成：ExportProject.ps1、Host ProjectFile/ProjectActions 和两种生成器的脚本
  部署；源目录/输出重叠在 UI 和导出器均拒绝。用户指定输出不得进入项目文件，
  已实现本机 QSettings 偏好和独立必填 CLI -Output，附带 .gproject 示例。
  导出锁改为系统 DeleteOnClose，取消/进程退出后释放，旧指针保持完整；异常
  staging/临时输入清单可能保留，不自动删除历史/用户文件。测试使用独立临时
  QSettings，不修改用户设置。
  验证：CMake/Premake Debug/Release Host 的实际设置对话框/菜单、异步导出、
  预览、零复制、失败/取消/重试与本机输出重开均通过，既有诊断恢复均通过。
  显式 ExportGame 回归通过。用示例 .gproject 导出实际已有 CMake Release
  二进制，重复 0 复制；OpenGL/Vulkan/D3D12 分别 120 帧、全部三项读回通过，
  正常退出 0。证据在本轮 Codex 工作目录 work/editor-project-*.json 和
  editor-project-state 内的运行日志；未重建引擎，不代表源码/Debug 游戏验证。
  输出仍是原始资源目录，S2/S4 总项、Mono、资源格式转换、.pak 和完整许可/
  原生依赖扫描保持开放。未提交/推送。

- [x] S2.2 PowerShell 导出与真实示例闭环（2026-10-26）：将独立导出工具迁移至
  仓库现有 PowerShell 工具链，移除 Python 运行依赖；提供 FrameGraphTriangle
  文件清单生成和包启动器，启动器不依赖调用 cwd，运行配置/缓存写入独立用户
  工作目录，避免引擎退出重写 Config.ini 破坏不可变版本。验收保留 S2.1 回归、
  实际示例完整/重复导出、从仓库外启动的有界 GPU 读回与正常退出，逐后端记录。
  脚本/通用场景导出与许可证自动审计仍待后续实现，不在本子项宣称完成。
  完成：ExportGame.ps1 替换初版 Python 工具/测试；ExportFrameGraphTriangle.ps1
  收集现有构建文件，包内 LaunchGame.ps1 验证所有哈希并创建独立可写会话。
  子进程只使用包/系统 PATH，清除继承的 GENGINE/VK/MONO 覆盖；Config.ini、
  Assets/cache 和日志均位于会话目录，发布版本保持不变。每次会话使用包默认
  配置，不保留跨会话设置/缓存；会话目录/旧版本保留供检查，不自动清理。
  验证：TestExportGame.ps1 通过预览/依赖环/单资源增量/零复制/时间戳、发布
  指针失败与重试、锁、路径/冲突/依赖/重叠/脚本拒绝、junction、资源移除、
  用户文件/历史保留和损坏检测。TestExportedFrameGraphTriangle.ps1 使用已有
  CMake Release 二进制，在本轮 Codex 工作目录 work/package-final-verification
  创建包（39 文件，首次 201334674 字节），重复和运行后导出均 0 复制。
  OpenGL/Vulkan/D3D12 分别完成 120 帧、全部三项 GPU 读回、正常退出 0，
  stderr 为空，捕获 stdout 无 error/VUID/assertion。证据 verification.json
  与各后端会话 stdout/stderr.log。脚本改变后已重新导出并运行全部三后端。
  未重建引擎，本项验证已有二进制的部署，不宣称当前源码重新构建或验证。
  未覆盖 Debug/Premake 包、干净机器/长期内存、Mono、通用场景/原生依赖
  发现和厂商许可审计；示例包仅用于本地验证，不作为完整可再分发产品。
  架构/API/指南/示例 README 已同步；未提交/推送。

- [x] S2.1 显式清单目录导出器（2026-10-26，历史初版，已由 S2.2 迁移至 PowerShell）：新增标准库 Python CLI，支持
  文件/资源显式依赖闭包、预览、SHA-256 增量、隔离版本发布与原子 current 指针。
  拒绝路径逃逸、链接、输入输出重叠、缺失依赖及大小写冲突；失败保留上一版本，
  不删除历史版本或用户文件。验收隔离回归覆盖首次/重复/单文件变更、资源环、
  无效输入和发布失败。此项仅提供部署基础，不宣称自动场景解析、Mono 兼容
  验证或可运行完整游戏；S0/S1/S2/S4 总项保持开放。
  初版完成（对应 Python 文件现已由 S2.2 替换）：导出工具、示例 JSON 与 GameExport 指南；显式动态根、
  依赖环去重、逐文件计划、内容寻址版本及不可变旧版本硬链接复用。首次复制、
  重复零复制且时间戳不变、单资源更新仅复制一项；不硬链接可变源文件。
  历史验证：初版 Python 的 8 项隔离回归通过，
  包括复制/指针发布故障保护、历史/用户文件保留、输入链接拒绝、路径/依赖/
  冲突/重叠校验、损坏检测及并发锁拒绝。源/输出路径为可信目录，外部进程竞态
  不在安全边界内；要求输出文件系统支持硬链接，发布文件必须不可变。
  仅新增独立部署工具，未修改引擎或构建配置，无需重建引擎；未验证实际游戏
  启动、脚本包或三后端。剩余：运行时清单接入、原生依赖发现、资源格式引用
  适配及脚本 SDK 部署；S2/S4 保持开放。未提交/推送。

- [x] 2026-10-25 资源打包范围补充（用户要求，仅文档）：明确首版包括游戏资源
  的依赖收集与目录部署；.pak 索引/压缩/读取为后续独立阶段。验收：架构/API
  一致描述资源身份、启动场景与动态依赖、缺失资源、增量更新及包外依赖检查，
  不宣称已有资源导出器或归档读取器。
  完成：架构 7.1/7.2、API 资源契约、索引/构建指南已同步；明确 S4 为 S2
  首版导出的必要资源部分，S5 为后续归档。链接/内容与 diff 检查通过；无代码
  修改，不需引擎构建，功能实施仍保持开放，未提交或推送。

- [x] 2026-10-25 文档先行（用户要求）：设计开发共享 Mono、可配置脚本路径、
  发布增量部署和 Worker 内 C# 编译/重载边界；本轮仅写文档，不修改代码。
  验收：对照当前 ScriptEngine、RuntimeSpecification 与两种构建入口，区分
  已实现/拟议能力，写明 BCL 来源、路径优先级、打包清单、错误/资源生命周期与
  Debug/Release 验证矩阵，同步 Architecture/API 索引及使用指南。
  完成：Architecture/ManagedScripting.md 与 API/ManagedScripting.md 已写入，
  服务、构建、Qt 编辑器页面及索引/指南同步。对照源码确认静态原生库与缺失
  规范 BCL 来源的问题；文档链接与 git diff --check 通过。仅文档，无代码、
  构建或部署行为变化，不需重建引擎；S0–S3 保持开放，未提交或推送。

- [ ] S0 路径与初始化：建立显式的脚本启动配置和路径解析，开发共享固定版本
  Runtime SDK，发布只使用包内清单。初始化预检、异常传播和部分失败清理覆盖
  Debug/Release，不依赖工作目录，不在断言中执行必要操作；关闭脚本时不查找 Mono。
- [x] S0.1 显式脚本路径与健壮初始化（2026-10-26 跟进）：在
  RuntimeSpecification 中加入 ScriptRuntimeRoot / ScriptCoreAssembly，EngineRuntime
  构造时将相对路径解析为绝对路径并把结果传给 ScriptEngine；ScriptEngine
  校验路径、处理域/程序集/方法为空、记录失败并清理半初始化状态。验收：
  Debug/Release 构建；显式路径与默认路径均可编译；路径或程序集失败不继续
  解引用；关闭脚本时不调用 Mono 查找。清单、RuntimeId、Development/Packaged
  模式和 Worker 重载不在本项实现。
  完成：RuntimeSpecification 新增 ScriptRuntimeRoot 与 ScriptCoreAssembly；
  EngineRuntime 在切换 cwd 前按 WorkingDirectory 解析相对脚本路径并记录最终
  路径。ScriptEngine::Init 改为接收 RuntimeSpecification 并返回 bool，校验
  runtime root/core assembly，处理 root/app domain、assembly image、class、
  method、instance、Mono string 为空及托管调用异常；失败时清理已创建的
  domain/runtime，避免半初始化状态和断言-only 检查。
  验证：CMake Debug 完整编辑器/引擎构建、协议/恢复/真实引擎检查通过；
  CMake Release GEngine 构建通过。证据：
  build/qt-editor/windows-x86_64/Debug/{engine-build.log,tests.log,engine-tests.log}
  与 build/cmake-vs2022/windows-x86_64/Release/GEngine/GEngine.dll。
  未验证：启用脚本的真实 GUI 运行与故意路径/程序集失败进程退出码；S0 的
  RuntimeId/清单/Development/Packaged 模式仍开放。
- [ ] S1 运行库与编译：核实当前静态 Mono 与托管 BCL 的兼容版本及许可证，
  建立单一 SDK 清单；构建 ScriptCore/游戏程序集，CMake/Premake 消费相同产物。
  不将“所有 Mono DLL”或任意另一个项目的 mscorlib 当作经过验证的运行库。
- [x] S1.0 Runtime SDK 审计（2026-10-26 跟进）：盘点当前静态 Mono 库、
  示例中的托管 BCL 副本与许可证/来源信息，比较主要程序集版本和内容哈希。
  验收：明确是否存在可复现、可再分发且与静态库匹配的 BCL 集合；记录差异、
  证据与阻塞项，不得据此实现路径切换或宣称 SDK 已经验证。
  完成：Sandbox 与四个示例的 mono/lib/mono/4.5 是同一 445 文件副本，
  每份 80,109,965 字节；mscorlib/System 等程序集版本为 4.6.57.0，
  mscorlib SHA256 为
  C7512C133808ACB42E48B6C1B879AB6F3C988A9E7A83B6A80E75A9FF38B6E5CF。
  Debug/Release 静态
  libmono-static-sgen.lib 与 Hazel 提交 b22a7550 的同名文件逐字节一致，
  但示例 BCL 与 Hazel 当前 Hazelnut 副本不同。本地 BCL 最早由主仓库提交
  94835394（2024-07-23，update:all）引入，提交未记录上游来源。
  官方 Mono 6.8.0.105、6.10.0.69、6.10.0.87 x64 MSI 已下载/管理解包比对；
  三者的静态库与本地 Debug/Release 均不同，4.5 mscorlib 也没有本地哈希。
  第三方 LICENSE 与 Hazel 提交一致，说明运行时/类库主要为 MIT 与若干
  BSD/Apache/MPL/GPL-with-exception 组件；LICENSE 引用的 PATENTS.TXT
  未随快照提供。
  结论：当前不存在可复现、可再分发且与静态库已验证匹配的 BCL SDK。
  S1 仍阻塞在确定可再分发来源/许可证与构建配对；不得把示例目录直接提升
  为正式 Runtime SDK。Architecture/API 文档与本 TODO 已同步；本轮仅文档，
  无需引擎构建。
- [x] S1.1 官方 Mono SDK 替换（2026-10-26 用户要求）：新增可重复脚本下载
  官方 Mono 6.10.0.87 x64 MSI 并管理解包到 dist/mono；CMake/Premake 的
  include/lib 输入改为该官方 SDK，不再消费 Hazel 静态库快照。验收：脚本
  校验 MSI 哈希并输出 SDK 路径；GEngine Debug/Release 至少一配置完成链接；
  文档明确运行时 BCL 路径仍属 S0，未宣称脚本路径/打包已完成。
  完成：BuildScripts/BuildMonoSdk.ps1 下载/复用 MSI，校验 SHA-256
  8853A0F1…9D0FE4，管理解包到
  dist/mono/windows-x86_64/6.10.0.87/Mono，并从官方 mono-6.10.0.87
  标签获取 LICENSE/PATENTS.TXT 与 mono-sdk.json 清单。CMake/Premake 已改用
  include/mono-2.0 与官方 libmono-static-sgen.lib；BuildQtEditor.ps1 的完整
  构建会自动准备该 SDK，-ProtocolOnly 不下载。
  验证：CMake Debug 完整编辑器/引擎构建、协议/恢复/真实引擎检查通过；
  CMake Release GEngine 构建通过。证据：
  build/qt-editor/windows-x86_64/Debug/{engine-build.log,tests.log,engine-tests.log}
  与 build/cmake-vs2022/windows-x86_64/Release/GEngine/GEngine.dll。
  边界：运行时仍使用 cwd 下 mono/lib 与 Resources/Scripts/GEngine-ScriptCore.dll；
  未运行启用脚本的 GUI 程序，未验证 BCL/程序集兼容，S0/S2 保持开放。
- [ ] S2 增量导出：预检输入与依赖闭包、预览清单；按内容哈希跳过不变文件，
  隔离 staging 后发布，限制清理为旧清单拥有的路径，校验包外路径/缺失依赖。
  导出到新目录后脱离开发 SDK、改变工作目录仍可运行；第二次无变化导出零复制。
- [ ] S3 编辑器 C#：Qt Host 触发外部编译并展示结果，只有 Worker/游戏加载 Mono。
  首版停止旧 Worker、创建新进程后加载程序集；失败保留 Host 文档，可回退到
  最近一次完整构建。场景快照/运行状态保留及进程内热重载不在首版实现范围。

- [ ] S4 资源收集与目录部署（S2 首版导出的资源部分）：由启动场景/显式根资源
  递归收集场景、模型、纹理、材质、着色器、音频及其他受支持资产的依赖闭包；
  动态加载依赖显式声明。以稳定资源身份和包内路径连接引用，缺失、冲突或
  包外路径预检失败；导出后脱离项目源码可加载，单资源变化仅更新受影响内容。
  在 S2 首版导出验收前完成所需资源类型的 S4 子集，不能只部署 EXE/Mono。
- [ ] S5 可选 .pak 归档（后续）：沿用同一资源清单/身份，设计版本化索引、
  压缩与运行时读取适配，验证目录与归档加载等价、损坏检测及资源生命周期。
  首版不承诺加密、增量补丁包、流式加载或任意格式的自动资源转换。
  S5.1 最小 .gpak 已完成，格式/原生展开/完整性及路径验证见上方 S2.4。
  此总项继续跟踪稳定身份、压缩、直接读取适配等，不能再将所有归档能力视为未实现。

当前证据：ScriptEngine 使用显式 ScriptRuntimeRoot/ScriptCoreAssembly 路径，
仍加载固定 GEngine.Main 测试类；Mono 原生库静态链接，构建已切换到官方 SDK。
旧示例 BCL 无法追溯到匹配官方包，不可作为已验证 SDK；当前 Qt Worker 禁用脚本。
S2.1/S2.2 已验证显式清单部署和关闭脚本的三角形示例包；完整游戏/脚本包仍未完成。
