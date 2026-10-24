# 编辑器架构重设计

## 设计文档（2026-10-24）

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

## 实现阶段（均为计划，须另行授权）

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

- [ ] E1：拆出 EditorSession、SceneDocument、SelectionService、PanelHost 和
  ProjectContext，先保持单文档与既有文件格式。验收：新建/打开/保存/取消、
  失败不覆盖旧文档、脏状态提示、选择在关闭/删除后失效；现有面板可用。
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
- [ ] E5：建立项目路径、AssetService、文件操作结果和诊断面板。验收：项目
  切换不依赖进程工作目录；失败/取消不破坏文档；异步导入的过期结果被丢弃，
  GPU 创建遵循设备线程约束，退出时任务可取消并完成回收。
- [ ] E6：收敛兼容层与构建依赖。验收：EditorLayer 仅适配生命周期，面板
  不直接提交 GPU 工作、不持有裸 Scene 指针、不访问 EnTT 私有注册表；
  CMake/Premake 均构建，示例与手动验证矩阵记录完成/阻塞项后再删除旧路径。

配套设计：[架构](../Documentation/Architecture/Editor.md)、
[接口契约](../Documentation/API/Editor.md)。既有 Gizmo 引擎转发工作仍由
[依赖 TODO](Build/ThirdPartyDependencies.md#planned-engine-owned-gizmo-forwarding-interface)
管理，不在此重复创建独立实现。
