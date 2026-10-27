# C# 路径与增量打包契约

日期：2026-10-25。状态：**部分契约已实现；2026-10-26 S1.0 审计未找到可验证
匹配的 Runtime SDK，S1.1 已切换构建输入到官方 Mono 6.10.0.87 SDK，S0.1
提供显式脚本路径与健壮初始化；S2.4/S5.1 已提供原生自动编译与 .gpak，完整托管打包仍未完成**。
配套：[架构](../Architecture/ManagedScripting.md)、
[实施与验收 TODO](../../TODO/ScriptingAndPackaging.md)。

## 当前可调用接口

`ScriptEngine::Init(const RuntimeSpecification&)` / `Shutdown()` 和
`RuntimeSpecification::EnableScripting` 为现有接口。`ScriptRuntimeRoot` 与
`ScriptCoreAssembly` 是显式路径字段；`EngineRuntime` 在切换 cwd 前解析相对
路径。下列名称表示尚未实现的 RuntimeId/清单/发布模式，不是已存在的
C++ 类型、配置字段、环境变量或命令行参数。
构建输入已切换到官方 Mono 6.10.0.87 SDK；这未改变脚本运行时路径或导出能力。

## 已实现：S2.1 目录部署 CLI

S2.4 当前项目入口为 `GEngineTools.exe export-project --project <file.gproject> --output <directory>`，
自动编译并输出 EXE/.gpak/DLL。`.gproject` 无任何打包信息，Qt 窗口仅选输出路径，
不选择程序或构建选项。Host 只把输出作为本机 QSettings 偏好。下述目录 CLI
为仍保留的低层原型，编辑器不再调用它。当前接口见 [Qt 项目契约](QtEditor.md)。

以下 S2.2 目录部署契约是历史记录；对应 `.ps1` 工具已随原生
`.gproject`/`.gpak` 管线移除，不再作为当前入口：

`pwsh -File BuildScripts/ExportGame.ps1 -Manifest <json> -Output <directory> [-Preview]`

输入 schema 1 包含 `enableScripting: false`、`roots` 路径映射、`entrypoint`
文件 ID、`include` 动态根 ID 数组及 `files`。每个文件含唯一 `id`、`root`、
相对 `source`、包内 `path`、可选 `dependencies` ID 数组，以及 `role`
（native/asset/config/license，默认 asset）。entrypoint 必须为 native。
显式依赖由调用方保证完整；工具不解析场景/原生 DLL 引用或转换资源格式。

输出 PowerShell 对象（可转为 JSON）包含 releaseId、files、added/updated/reused/removed、copyFiles/
copyBytes、逐文件 plan 和 removedPaths。退出 0 表示成功，错误退出 1 并写 stderr。
Preview 不创建目录；copy 计数仅统计 payload，不含生成清单/指针。
受管理输出使用 `releases/<releaseId>/package-manifest.json` 和 `current.json`。
发布清单不含源机器绝对路径；版本 ID 为实际发布清单 UTF-8 字节的 SHA-256。
重复导出校验已有文件，零 payload 复制；变更版本硬链接旧版本内未变文件，
旧版本不可修改。历史版本/用户文件保留，当前版本通过原子指针切换。

该 CLI 不提供 `game-runtime.json` 或引擎 Packaged 模式，不支持启用脚本的包。
C++ 公共接口和后端行为无变化。完整示例、路径限制、lock/失败恢复及运行边界
见 [导出指南](../Guides/GameExport.md)。下文其余契约仍为目标能力。

S2.2 历史示例同时提供 `ExportFrameGraphTriangle.ps1`，参数为 `-Configuration <Debug|Release>`、
`-Generator <CMake|Premake> -Output <directory> [-Preview]`，打印 JSON 计划/结果，
收集现有构建目录的 EXE/DLL、配置、两个 shader 和包内 `LaunchGame.ps1`。
包内启动器支持 `-StateRoot`、`-GraphicsAPI <Default|OpenGL|Vulkan|D3D12>`、
`-FrameLimit` 与 `-TimeoutSeconds`；先校验完整性，再复制资源/配置到独立可写
会话目录，从包内启动原生程序并记录退出码/日志。不改写发布版本，不依赖调用
cwd 或开发 SDK PATH。运行依赖系统图形驱动/运行库与 PowerShell 7.2+；不是完整
原生导入/许可证扫描器或通用引擎 Packaged API。构建配置及三后端公共接口不变。

2026-10-26 审计结论：仓库内五份示例 BCL 副本彼此一致，但与本地静态库
没有已验证配对，也与已比对的三个官方 Mono x64 包不同；静态库为 Hazel
二进制快照。RuntimeId/清单输入不能指向这些副本，S1 需先建立可复现、
可再分发且经过兼容性验证的 SDK。

## 拟议启动输入与结果

| 输入 | 契约 |
| --- | --- |
| Mode | 显式 Development/Packaged；Legacy 仅作为显式迁移选项。 |
| RuntimeId | 匹配已核验的原生 Mono、架构、托管 profile 和内容清单。 |
| RuntimeRoot | 运行库根目录，由 SDK 清单声明 Mono assemblies/config 实际子路径。 |
| CoreAssembly | ScriptCore 产物路径，与 RuntimeRoot 独立。 |
| ProjectAssembly/Dependencies | 项目脚本及依赖的完整构建清单；未实现通用托管组件绑定。 |
| BuildId | 最近一次完整构建身份；不能加载混合构建版本。 |

Development 优先级：显式配置 → 项目本地启动清单 → 开发环境覆盖。显式来源
无效则报错；环境覆盖只接受绝对路径，清单相对路径以清单目录解析。
Packaged 只使用 EXE 旁 game-runtime.json，路径相对包根，拒绝逃逸/外部开发
SDK 兜底。EnableScripting=false 时不解析这些输入。

拟议初始化结果区分未启用、就绪、缺失文件、Runtime ID 不匹配、清单无效、
域创建失败、程序集加载失败和托管异常。日志包含最终选中路径及失败阶段；
Debug/Release 使用相同必要检查，不以断言代替错误处理。
配置对象不传递 MonoDomain/MonoImage 等原生指针给 Qt Host。

## 拟议导出输入与结果

输入包括游戏原生构建清单、脚本 Build ID、运行库清单、资产/依赖、目标平台/
配置和受管理输出根。源、SDK 与输出必须不重叠。首版先提供计划检查，再执行
部署；S2.1 已实现上述显式清单 CLI，运行库/自动资产清单仍待实施。

部署清单记录规范化包内路径、内容哈希、大小、角色及运行库来源/许可证身份，
不记录机器绝对路径。游戏清单标识脚本开关、运行库身份和程序集入口。清单 schema
必须带版本，未知版本拒绝处理；字段定义和真实示例在实现时与解析器一起提交。

结果包含新增/更新/复用/删除文件及字节计数。以内容哈希跳过不变项，默认不部署
编译工具；未启用脚本时不部署托管 BCL。依赖集合包含动态加载所需的显式条目，
不能仅靠扫描静态引用裁剪。只删除旧有效清单拥有的文件，保留用户文件。

失败不发布半成品；先写 staging、校验完整性，再发布受管理版本并原子切换指针。
运行中包不原地覆盖。不把原生 Mono 已静态链接误解为无需托管程序集。

## 拟议资源清单与存储边界

首版导出包含场景、模型/网格、纹理、材质、着色器、音频等受支持资源的
目录部署。输入为启动场景/显式根资源、动态加载声明和目标变体；输出为去重
依赖闭包及资源部署计划。缺失引用、冲突、包外路径或未支持类型返回失败。

资源清单拟包含稳定逻辑身份、类型、规范化包内路径、依赖、内容哈希和目标
变体信息。现有引擎尚无统一资源 ID/读取层，此处是目标契约；实现需提供可
持久保存的映射，并处理已有文件中的路径引用。不得在场景中存储归档物理偏移。

目录模式先部署到 Assets/，使用与运行库相同的内容哈希增量及清单所有权规则。
派生资源仅在输入或相关构建配置改变时更新，目标后端/平台不兼容时不得复用。

S5.1 提供最小 .gpak 写入，S5.2 已提供原生校验/挂载与条目直接读取，见 Runtime API。
资产数据库身份、压缩和归档块级增量仍为后续 S5。
文件级零复制不等于归档块级更新或增量补丁已实现。
S5.2 将逻辑资源路径解析为归档条目并返回独立持有的字节，同时为 Config/
日志/cache 提供包外可写目录。Config/Config.ini 和 Cache/Shaders/<API>/ 均位于
游戏根目录并跨启动保留，配置不进归档。`PreparePackagedResources` 校验并挂载索引，
`ReadResource` 按条目读取，不再展开 Assets；这不实现 C# 运行库部署。
S5.3 已实现根目录仅含 `<Game>.exe` 与 `Runtime/`、`Data/`、`Metadata/`
分类子文件夹。根 EXE 是启动器；真实游戏 EXE 与原生 DLL 位于 `Runtime/`，
`.gpak` 位于 `Data/`，发布清单位于 `Metadata/`。这不是把载荷嵌入 EXE 的
单文件 API。

## 编辑器与兼容边界

Host 编译请求和日志展示不加载 Mono；只有 Worker/游戏执行 C#。第一版通过新
Worker 进程加载新 Build ID，编译失败保留上一份完整产物；完整场景状态恢复待实施。
引擎公共接口、C# 目标框架和现有构建命令本轮不改变；自动资产/脚本打包尚未
完成。验收按 TODO 的 S0–S5 分阶段提供构建、运行与部署证据。
