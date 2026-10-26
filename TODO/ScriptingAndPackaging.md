# C#、Mono 与游戏打包

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

当前证据：ScriptEngine 写死 mono/lib 与 Resources/Scripts 路径，加载固定
GEngine.Main 测试类；Mono 原生库静态链接。第三方 mono/lib 只有 Debug/Release
原生库目录，未发现 mscorlib.dll；示例下存在旧托管运行库副本。当前 Qt Worker
禁用脚本。S1.0 已确认示例 BCL 彼此一致但无法追溯到匹配的官方包；静态库
来自 Hazel 二进制快照。以上不是路径/打包新能力的实现或运行验证。
