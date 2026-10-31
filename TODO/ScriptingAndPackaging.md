# C#、Mono 与游戏打包

- [x] Native C++ scene scripts (2026-10-31, user authorized): implement C++ as
      a parallel scene-script language.
      GameObject should be able to attach either a managed C# ScriptBehaviour
      or a native C++ NativeScriptBehaviour by class name. Scope for the first
      bounded implementation: define the native behaviour base class and
      factory registry, keep scene serialization unchanged (`Scripts` remains
      a class-name sequence), dispatch the same lifecycle phases, and allow a
      focused example to register and run both C# and C++ behaviours. Do not
      resurrect the removed legacy NativeScript component. Acceptance:
      Debug/Release CMake and Premake engine plus focused example builds pass;
      mixed C#/C++ lifecycle, failure handling and teardown are exercised;
      documentation states that C++ scripts are statically registered in the
      current milestone and not dynamically loaded source files.
      Implemented: `NativeScriptBehaviour` provides the same lifecycle
      contract, `ScriptEngine::RegisterNativeScript` owns a class-name factory
      registry, and `GE_REGISTER_NATIVE_SCRIPT` statically registers a
      game-side C++ behaviour. The GameWorld example now attaches and runs
      both C# and C++ behaviours, including a native failure path and teardown.
      CMake and Premake Debug/Release builds and runs passed. Remaining:
      dynamic source loading, editor authoring/compilation, project assemblies,
      native-only GE_WITH_MONO=OFF support, and managed Transform/API parity
      are not implemented in this milestone.
      A GE_WITH_MONO=OFF engine build compiles, but native script dispatch
      remains unsupported in that configuration.

2026-10-31 scene world-logic C# dispatch is owned by
[SceneAndPhysics](SceneAndPhysics.md): GameObject script names round-trip in
`.gscene`, ScriptEngine owns `ScriptBehaviour` instances and dispatches their
lifecycle/update phases. Native C++ behaviours are now supported through
static registration, while dynamic source loading, project assemblies,
Transform/engine APIs, editor compilation and packaging remain owned by the
S0–S5 items below.

## Managed dynamic-loading milestones (planned, 2026-10-31)

- [ ] D1 Startup project-assembly loading. Extend script startup to accept an
  explicit ordered set containing `GEngine-ScriptCore.dll`, the game assembly
  and declared managed dependencies. Preflight paths, hashes, architecture,
  target framework and dependency closure; load them into the existing Mono
  app domain before resolving scene script classes. Record the final assembly
  identities and hashes in diagnostics. Acceptance: a focused example attaches
  a class from a separately built project assembly, missing/duplicate/circular
  dependencies fail before domain creation, and startup/normal shutdown pass in
  Debug and Release. No unload or reload is included.
- [ ] D2 ABI compatibility and Build ID contract. Add a versioned ScriptCore
  ABI marker and require each project assembly to reference it. Build outputs
  record Build ID, ScriptCore hash, project assembly hashes, dependency hashes
  and target framework. Reject mismatched or stale combinations before any
  scene state changes. Acceptance: compatible and deliberately mismatched
  project assemblies are covered by focused regressions, and error messages
  identify the failing artifact/version.
- [ ] D3 Editor reload via Worker restart. Qt Host compiles a new Build ID,
  validates it, stops the old Worker, starts a new Worker with that Build ID,
  and restores the Host-owned scene snapshot. Compilation or load failure
  preserves the current documents and keeps the last complete Worker/build
  available. Acceptance: edit/compile/restart/reload and failure-rollback flows
  are covered, including cancellation and repeated reload. This is the first
  reload path; it does not use in-process domain unload.
- [ ] D4 Optional in-process app-domain reload. Only after D1–D3 are stable,
  design a bounded unload/reload path: quiesce script dispatch, destroy all
  script instances and wrappers, free every GC handle, stop managed threads,
  verify no live domain references, unload the app domain, load the next Build
  ID and rebuild script instances from an explicit state snapshot. Define
  serializable script state and failure behavior for non-serializable fields.
  Acceptance: repeated in-process reload, exception cleanup and leak/handle
  checks pass; otherwise keep this milestone closed and continue using D3.

These milestones explicitly exclude loading arbitrary untrusted source files
at runtime and do not make the Qt Host load Mono. Security validation and
package-time assembly verification remain part of the export/manifest work.

## GAsset tooling entry point (2026-10-29)

- [x] Native C++ correction: replace Python dispatch with C++ conversion and
  reference repair; remove the script and interpreter option. Acceptance:
  Debug/Release tool builds and isolated text/binary/scene/collision/repair
  regressions pass without Python at runtime. Preserve resource scope and schema;
  document implementation and any remaining migration limitations.
  Implemented: conversion/discovery, Base64/YAML emission, collision suffixes,
  scene migration and reference repair now run in GEngineTools.cpp. Removed
  ConvertResourcesToGAssets.py and the gasset --python option. Text scalars retain
  exact UTF-8 whitespace instead of normalizing tabs/trailing spaces. Invalid
  UTF-8 and collisions fail before writes; originals are removed after successful
  output/reference writes. Runtime APIs and GPU ownership are unaffected.
  Verification: CMake GEngineTools Debug/Release builds passed; both binaries
  passed Tests/GAssetTools.py with an empty child PATH (no Python process lookup),
  covering Unicode/CRLF/tab text, all byte values, empty payloads, same-stem
  collisions, scene/reference updates, repair, repeat runs, exclusions, rejected
  interpreter options and preflight failures. Harness requires Python/PyYAML;
  the actual tool does not. CMake/Premake source lists need no change because
  implementation remains in their existing GEngineTools.cpp source.
  Remaining: batches are not atomic on I/O failure and preflight retains payloads
  in CPU memory. No renderer changes or backend runtime runs were required.

- [x] Historical wrapper integration (superseded by the native C++ correction above).
  Scope: reuse the existing Python migration implementation through the native
  workflow tool, with explicit root/interpreter options and propagated failures.
  Acceptance: tool builds, conversion/repair work on an isolated resource fixture,
  invalid arguments and conversion errors fail, and architecture/API usage agrees.
  Runtime loaders and GPU interfaces are unaffected; a native converter rewrite
  is outside this bounded integration.
  Implemented: added the gasset command with help, root/interpreter validation,
  explicit Python PATH resolution, reference-repair mode and worker exit status.
  Existing CMake/Premake source lists already include the changed tool source.
  Verification: CMake GEngineTools Debug/Release builds passed. Both binaries
  passed isolated fixtures covering shader conversion, scene migration, reference
  rewriting/repair, paths containing spaces, invalid arguments/root, collision
  rejection and missing interpreter. Initial default interpreter lookup failed;
  resolved it through SearchTool and reran both configurations successfully.
  Documentation links/content reviewed; git diff --check passed. No runtime or
  rendering code changed, so engine/example GPU runs were not required.
  Historical limitation (now removed): conversion required Python and the source checkout.

2026-10-28：编辑器 E1.7（EditorArchitecture.md）新增可选
settings.defaultScene，旧 schema 1 描述兼容。原生导出器接受该项目启动元数据，
不改变游戏原生入口或本机打包偏好；Debug/Release 元数据回归及 Release 实际
打包/运行通过，证据见 E1.7。以下旧条目中的 name/version-only 为当时历史范围。

## Engine-specific resource data types (planned, 2026-10-28)

- [ ] Define engine-owned resource data types and their serialized asset format.
  Scope: distinguish source/import formats (PNG, FBX, etc.), engine CPU resource
  data, serialized runtime assets, and backend GPU objects. Decide which types
  need dedicated schemas (initial candidates: texture, mesh, material, shader),
  with explicit resource type, schema version, stable identity, dependency
  references, payload layout and validation rules. Names/extensions and the exact
  public API remain undecided; do not introduce a generic byte wrapper alone and
  call the asset-format problem solved. `.gpak` remains a container for resource
  payloads, not the definition of each resource's data type.
  Begin with one bounded resource type and a source importer -> engine data ->
  directory/archive loader -> RenderDevice upload example. Preserve owning CPU
  data through asynchronous consumption/upload and keep native handles in the
  backends. Consider OpenGL/Vulkan/D3D12 capabilities explicitly. Keep mutable
  Config/Cache outside the archive and retain compatibility with existing readers
  during migration. Connect this design to S4 dependency collection rather than
  adding a separate resource identity system for packaging.
  Acceptance: documented CPU/file/GPU ownership boundaries; deterministic import
  and serialization; equivalent directory/.gpak loads; explicit rejection of
  unknown types/versions, malformed/truncated payloads and invalid dependencies;
  a focused example with resource lifetime and available-backend verification;
  synchronized architecture/API/import/export documentation. Record deferred
  resource types and migration gaps individually.
  Status: user requested TODO registration only. No implementation or new API is
  authorized by this entry. Documentation links/content reviewed; no engine build
  is required for this planning-only change.
  2026-10-29 user-authorized bounded follow-up: establish a two-extension
  authoring convention analogous to Unreal's `.uasset`/`.umap`: use `.gasset`
  for engine resources and `.gscene` for scenes. For the first bounded
  implementation, `.gasset` may reuse YAML as an envelope with explicit type,
  schema version, source format and payload fields; text payloads can be stored
  directly and binary payloads may use an explicit hexadecimal representation.
  This migration must not merely rename raw PNG/FBX/TTF/shader bytes while
  claiming an engine-owned asset format. First inventory loaders, editor
  filters, project references, archive collection and cache behavior; then
  either provide an explicit `.gasset` envelope/type identity for the bounded
  resource set or record the narrower compatible migration actually
  implemented. Preserve `.gpak` as a container and
  keep project/package files outside this resource-extension milestone.
  Acceptance: all first-party example references and shipped resource files use
  the selected convention; loaders reject malformed or unknown resource data;
  equivalent development and `.gpak` loads pass on available backends; scene
  open/save/export and shader cache behavior are exercised; documentation and
  editor filters no longer advertise mixed resource extensions. Remaining
  implementation, verification and migration gaps stay open in this entry.
      Implemented (bounded first pass, 2026-10-29): added the validated YAML
      GAsset envelope and public loaders; migrated first-party Example, Sandbox,
      editor Worker/editor Host runtime resources, scene references, filters and
      project metadata to `.gasset`/`.gscene`; added the migration tool with
      same-stem collision suffixes; connected texture, FBX, font, shader and
      shader-include paths; updated editor Host font loading and build copies;
      fixed FBX SDK child destruction order and Vertex hashing/Mesh copying;
      and documented the format in architecture/API/build/example pages.
      Verification: GAsset Debug/Release regression tests passed; CMake Debug
      and Release GEngine + FrameGraphTriangle builds passed; development-mode
      FrameGraphTriangle 120-frame resource checks passed on OpenGL, Vulkan and
      D3D12 (Debug Layer enabled); a generated `.gpak` runtime with external
      Config also passed all three backends, resource probes, three readbacks,
      archive-read checks and normal shutdown; `Tests/ResourcePackRuntime.py`
      additionally passed cold/warm cache reuse, include invalidation, corrupt/
      missing pack rejection and missing external-config rejection. Logs are under
      build/gasset-framegraph-release-api*.log and
      build/gasset-publish-api*.log. Existing unrelated compiler warnings
      remain. Full typed resource schemas, editor asset import/save workflows,
      Qt editor self-tests and a full `GEngineTools export-project` publication
      remain open; the direct exporter command still needs investigation after
      the manual package verification. Binary payloads also remain YAML Base64
      and should move to a compact envelope before large production assets are
      distributed.

- [x] External config and persistent shader cache correction (2026-10-27).
  User requirement: Config.ini must not be inside .gpak. Publish defaults under
  Config/Config.ini, keep mutable state directly in the game root (user correction),
  and verify that cache files are generated and reused on the second launch.
  Read/write that external configuration directly and preserve it on repackaging;
  separate backend caches and reject concurrent writers to one state directory.
  Preserve source config/backend selections. Acceptance: archive has no config,
  default/missing/changed external config and empty-resource projects behave
  explicitly, Debug/Release builds and available backend cold/warm runs pass,
  warm cache content is unchanged, packaging zero-copy/failure checks pass,
  and architecture/API/example documentation reflects the actual paths.
  Implemented: .gpak collection now contains only Assets/Licenses, with empty
  archives accepted for resource-free games. Config/Config.ini is published as
  a separate file (project config or generated defaults). Runtime uses the game
  root as cwd, reads/writes Config/Config.ini there, and retains a delete-on-close
  Config/.runtime.lock against concurrent state writers. No AppData or temporary
  state directory is used. Cache/Shaders/1, /2, /3 are persistent backend caches.
  Repackaging preserves destination config, Cache, Logs and UserData; mutable
  files are copied rather than hard-linked to keep old-version state independent.
  Missing destination config is restored on repackaging. Shader fingerprints now
  include recursive literal includes and ignore commented include directives.
  Verification: Debug/Release engine/game builds passed; each backend completed
  two 120-frame launches with three readbacks each. Each backend generated/reused
  8 nonempty cache files with unchanged content on the second launch. Both builds
  rejected missing external config and corrupt/missing packs before Application;
  modified include content invalidated its dependent cache. Source config stayed
  unchanged. Focused Debug/Release archive tests passed, including empty mounts,
  concurrency and handle checks. Release Host/tool built and all 3 editor tests
  passed; the packaging regression now modifies config/adds cache data, renames
  and republishes, and checks preservation. Manual generated-package checks also
  preserved all existing cache hashes and modified config, restored a deleted
  config from project defaults, then reported copyFiles:0 on unchanged export.
  One publication directory switch was temporarily denied by Windows; rollback
  preserved the old output and the later bounded retry passed. No atomic directory
  replacement is claimed. Generated defaults for a project lacking config were
  source-reviewed rather than building a separate fresh starter project.
  Macro-generated include dependencies, clean-machine/Premake builds, manual
  resize/visual checks and the existing D3D12 UAV-barrier defect remain outside
  this correction. Logs: build/gpak-root-config-logs and its Debug counterpart.
  Publication authorization (2026-10-27): user requested commit and push for this
  correction. Scope is the 25 reviewed implementation/test/example/documentation
  files; exclude tmp, build outputs, local backend config and submodule changes.
  Staged-diff/whitespace checks precede commit; verify commit, remote tracking
  revision and remaining worktree after push. No additional API/architecture
  behavior is introduced by this publication step.

- [x] 2026-10-27 用户授权提交并推送本机完成的自定义 EXE/Logo 修复与 S5.2
  直接归档读取工作。发布范围包括实现、示例资源、回归和同步文档；不包含
  tmp、生成工程/构建产物、本机配置或子模块指针。提交前检查预期文件清单、
  暂存差异和 diff --check；提交/推送后核对 HEAD、origin/master 与剩余工作区。
  此条记录发布授权与范围，不新增架构或 API 行为；功能验收证据见各所属条目。

- [x] Add executable name and logo/icon inputs to the editor packaging
      operation (2026-10-27). Scope: extend Project > Package project with
      executable-name and logo/icon fields, persist them as local packaging
      preferences rather than `.gproject` metadata, convert supported image
      logos to Windows `.ico` resources, pass both values to
      `GEngineTools.exe export-project`, and apply them to the categorized root
      launcher/game names and embedded launcher icon. Acceptance: the dialog
      exposes output, executable name, and logo; custom names produce
      `<Custom>.exe` at the package root and under `Runtime/` with matching
      `.gpak` and manifest identity; a custom logo is visible as the launcher
      icon; invalid names/logos fail before packaging; project metadata remains
      schema-1 name/version only; incremental repeat still reports zero copies;
      Host packaging regression covers the new fields; and documentation is
      synchronized.
      Implemented: MainWindow exposes output, executable-name and logo fields,
      persists them as local QSettings preferences, validates executable names and
      logo inputs, and renders supported images to multi-size Windows `.ico` files.
      `export-project` accepts `--executable-name` and `--icon`; the custom name is
      applied to the root launcher, runtime game, `.gpak` and manifest identity, and
      CMake embeds the icon in the launcher resource. PackagingCheck covers the
      three-field dialog, invalid inputs, custom identity and embedded icon.
      Verification (2026-10-27): CMake Release `GEngineTools` rebuilt successfully
      after the CLI changes, and `git diff --check` passed before documentation
      synchronization. The user interrupted further compilation, so Qt
      Host/PackagingCheck and an end-to-end custom package were not run. A prior
      `qt-editor --configuration Release --test` invocation stalled in its Ninja
      build with no active compiler process and was cancelled.
      Previously remaining: build the Qt Host, run PackagingCheck, perform an
      end-to-end package/icon check, and confirm incremental zero-copy behavior.
      Follow-up (2026-10-27, current user authorization): finish this bounded
      milestone by rebuilding Host/tool, running the real custom-name/logo
      packaging regression, inspecting the packaged icon and launcher behavior,
      fixing failures, and synchronizing architecture/API/example guidance.
      Preserve local backend selections and unrelated work. Record fresh evidence
      separately from earlier build results before closing this entry.
      Completed follow-up: fixed protected QTemporaryFile::open usage and switch
      case scope compile errors, included the complete temporary-file type for
      proper destruction, corrected ICO directory entries to 16 bytes, preserved
      empty logo preferences, cached icon bytes at a stable content-checked build
      path, and explicitly cleared the CMake icon argument when omitted.
      Fresh verification: CMake Release and Debug Host/tool builds passed.
      Each configuration passed EditorProtocol, EditorRecovery and the expanded
      EditorPackaging regression. The latter builds the real Release engine/game,
      checks native extracted blue/white icon pixels and matching custom identity,
      runs the custom root launcher from an unrelated cwd for 120 D3D12 frames
      with all three frame-119 readbacks and exit 0, checks incremental zero-copy,
      failure preservation and cancel/retry, then renames to `Renamed Game`, removes
      the icon, reopens preferences and verifies another unchanged zero-copy repeat.
      Release dialog screenshot visually reviewed: all three fields and action
      buttons render correctly. Project metadata and local backend config unchanged.
      Validation required an elevated test child because restricted access denied
      the system temporary output. The all-in-one qt-editor command's Mono setup
      failed to obtain/hash its installer; direct configured CMake builds passed
      and native game export excludes Mono. Fresh Mono setup is outside this item.
      Limits: OpenGL/Vulkan, manual Explorer icon-cache behavior and interactive
      resize were not rerun; this change does not alter renderer/backend APIs.
      Premake was not rebuilt for this follow-up. Documentation synchronized;
      final whitespace and diff review recorded before handoff.

- [x] 2026-10-27 用户授权提交并推送：发布本次项目文件、编辑器单输出路径
  自动编译、原生 .gpak 与相关工具/回归/文档。范围包含前置目录导出原型历史，
  不包含 tmp、生成的工程/构建产物、本机输出或配置；子模块指针不变。
  已复核本轮构建/运行证据与工作区差异，提交前检查暂存清单及 diff --check；
  推送后核对本地 HEAD 与 origin/master 一致。

- [x] Replace every first-party repository `.ps1` workflow with one native
      C++/Win32 tool (2026-10-27). Scope: remove all first-party
      `BuildScripts/*.ps1`, `BuildScripts/Tests/*.ps1`, and
      `GEngine-Editor/RunQtEditor.ps1`; add `GEngineTools.exe` with native
      subcommands for Mono SDK setup, Qt SDK build, Qt editor build/test,
      editor launch, `.gproject` export, resource-pack generation, publication,
      and packaging regression checks; update CMake/Premake generation,
      editor Host invocation, launchers, documentation, and README references.
      Third-party submodule scripts remain untouched. Historical directory
      export prototypes (`ExportGame`, `ExportFrameGraphTriangle`,
      `StartExportedGame`) are superseded by the native `.gproject`/`.gpak`
      pipeline rather than translated one-for-one. Acceptance: no first-party
      `.ps1` remains or is referenced as a current workflow; `GEngineTools`
      builds through CMake and Premake; `mono-sdk`, `qt-sdk`, `qt-editor`,
      `run-qt-editor`, and `export-project` commands work; editor packaging no
      longer requires PowerShell; `EditorPackaging` still covers the real
      build, `.gpak`, incremental repeat, failure preservation, cancel/retry,
      and close behavior; documentation accurately distinguishes native
      workflow tooling from third-party scripts; and `git diff --check` passes.
      Implemented: added `BuildScripts/Tools/GEngineTools.cpp` and generator
      targets in CMake and Premake. The tool provides `mono-sdk`, `qt-sdk`,
      `qt-editor`, `run-qt-editor`, and `export-project`; the Qt Host launches
      the deployed native exporter for Project > Package project. All fifteen
      first-party `.ps1` workflow/test files and the legacy deployed
      `packaging/*.ps1` payloads were removed. CMake and Premake both build and
      deploy `GEngineTools.exe`; `RunQtEditor.bat` locates and invokes it. The
      historical directory-deployment commands are documented as removed rather
      than translated one-for-one. Third-party submodule scripts remain
      untouched.
      Verification (2026-10-27): root CMake built `GEngineTools.exe`; `mono-sdk`
      reused the installed official SDK successfully; `qt-sdk --configuration
      Release` configure/build/install commands each exited 0; `qt-editor
      --configuration Release --test` configure/build/engine/CTest/engine-check
      commands each exited 0; Premake `qt-editor --premake` generated and built
      GEngineTools, Host, worker, diagnostic worker, and protocol tests; direct
      `export-project` compiled FrameGraphTriangle, wrote `.gpak`, published a
      flat output, and a repeat reported `copyFiles:0`; CMake and Premake Host
      `--packaging-self-test` runs both passed the real build, incremental
      repeat, failure preservation, cancel/retry, and close checks.
      `RunQtEditor.bat Release CMake -PrintPath` selected the CMake Host.
      Remaining limitations: fresh-machine Mono download/extraction and Debug
      tool commands were not rerun; GUI launch/interaction and clean-machine
      behavior remain separate checks; historical directory-deployment
      functionality is intentionally superseded rather than preserved.

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
  - [x] S5.2 直接读取 `.gpak`（2026-10-27 用户建议）：设计运行时资源读取
        适配层，在启动时解析并校验包索引，将规范资源路径映射到归档偏移，
        按需读取条目数据，而不是把整个包展开到 `%TEMP%` 的真实目录树。
        开发目录模式与打包模式必须使用同一逻辑资源身份；只读资源来自包，
        Config 重写、日志、shader cache 和用户状态写入独立可写目录，不得
        尝试修改 `.gpak`。迁移 Shader、材质、纹理、网格、配置等现有直接
        文件读取点，并保留数据生命周期直到上传/GPU/异步消费完成。验收：
        打包 FrameGraphTriangle 不再创建完整资源解包目录；三后端 120 帧
        运行与读回通过；缺失、重复、损坏、截断、路径穿越和索引越界均在
        读取/启动前明确失败；同一资源目录模式与归档模式加载等价；异步
        加载与资源生命周期有检查；文档区分当前 S5.1 解包行为与 S5.2
        计划。实现前需先审查现有资源读取路径和 GPU 数据所有权。
        Current implementation authorization (2026-10-27): finish S5.2 with a
        validated immutable archive index and owned per-read bytes, migrate shader,
        YAML/material/scene, image/cubemap and FBX input paths, keep configuration,
        logs and shader caches in writable session state, add focused malformed-
        archive/concurrent-read regressions, and run packaged FrameGraphTriangle
        on available backends without extracting Assets. Preserve existing custom
        executable/logo changes and backend config. Compression and a new async
        asset scheduler are outside this milestone; concurrent read ownership and
        existing upload data consumption must be checked explicitly.
        Failures found during integration: trailing cache-directory separators
        needed normalization; GProjectPackage must copy updated engine DLLs even
        when the game does not relink; FBX streams need an explicit reader ID;
        the RGB image probe exposed unsupported Vulkan RGB8 uploads. Normalize
        Vulkan file images/cubemaps to RGBA8 and verify the probe on all backends.
        Implemented: immutable validated path/offset/size index with retained
        read-only backing handle, atomic mount replacement and shared snapshots;
        per-read owning vectors with serialized seeks; FileSystemHelper reads,
        metadata/enumeration and write guards; YAML scene/material/config/cache,
        DXC includes, stb images/cubemaps, owned ImGui font copies and FBX memory
        streams. Only Config.ini is seeded, with Cache/Shaders, Logs and UserData
        outside the archive. Packaged runtime ignores development cwd overrides.
        GPU upload ownership and queue completion policies remain unchanged.
        Added ResourcePack CMake regression and ResourcePackRuntime.py matrix;
        FrameGraphTriangle exercises PPM upload, the existing GpuDriven cube as
        an FBX fixture, empty scene YAML, compute HLSL include and background bytes.
        PPM fixture disables text conversion to preserve binary pixel layout.
        Verification: Release/Debug focused tests passed 18 malformed archives,
        200 concurrent reads, directory/archive byte equivalence, immutable-file
        and write guards, failed-mount preservation, returned-buffer lifetime and
        100 remounts (Debug handle count 60 -> 60). Release and Debug engine/game
        builds passed with existing warnings; each OpenGL/Vulkan/D3D12 package
        completed 120 frames, all three readbacks, resource probes and normal exit,
        with no Assets tree after shutdown. Missing/corrupt packs exited 1 before
        Application construction in both configurations. Release EditorProtocol,
        EditorRecovery and expanded EditorPackaging passed, including custom icon,
        name, real runtime probes, incremental zero-copy and cancel/failure checks.
        A normal Debug development-directory D3D12 run also passed the probes and
        120-frame readbacks; its original Config.ini bytes were restored.
        D3D12 debug-layer-enabled development run reproduced the existing in-flight
        cross-queue UAV-barrier diagnostics tracked in Rendering/FrameGraphCore.md;
        this resource milestone does not close that synchronization defect.
        Limits: dedicated font/cubemap runtime scenarios and full material restore
        were not exercised (adapters compile; existing material semantics remain
        incomplete). No new async scheduler, compression, C# deployment, clean-
        machine or Premake rebuild, manual resize or long-duration memory claim.
        Runtime logs: build/gpak-runtime-logs, build/gpak-runtime-debug-logs and
        build/gpak-development-runtime.log. Final write-guard rebuild/rechecks and
        diff/link review are completed before handoff; unrelated tmp is preserved.
  - [x] S5.3 单入口分类发布布局（2026-10-27 用户要求）：将项目打包输出从
        平铺的 EXE/DLL/.gpak/.json 目录改为“根目录只有一个 `<Game>.exe`
        和若干分类文件夹”。原生 DLL、`.gpak` 与发布元数据不嵌入 EXE，
        而是分别放入固定子目录（例如 Runtime/、Data/、Metadata/；最终
        命名可在实现时确定）。先审计 PE import 依赖与启动顺序，并设计
        子目录 DLL 加载策略（如 delay-load + 显式搜索目录、动态加载或
        最小启动器）；不能假设 Windows loader 会自动搜索 EXE 子目录。
        发布清单、校验、版本与更新元数据移入 Metadata 子目录，不再在
        根目录输出 `.gengine-output.json`。系统图形驱动、Windows SDK/VC
        运行时等外部系统依赖仍可保留为机器前提，但需明确列出。验收：
        输出根目录仅包含 `<Game>.exe` 与子文件夹，除该 EXE 外没有任何
        散落文件；所有 DLL/.gpak/.json 均位于分类子目录；重复打包保留
        布局且只更新变化内容；从任意 cwd 启动根 EXE；三后端 120 帧/
        读回/退出 0；Config、日志、shader cache 与用户状态写入包外用户
        状态目录；损坏/篡改/缺失子目录载荷在启动前或首次加载前明确失败；
        Debug/Release 与干净机器检查通过；架构/API/导出文档同步布局和
        仍需的系统运行库。实现前需完成依赖/链接与目录契约设计，不把
        现有平铺输出伪装成单入口分类布局。
        Implemented: project CMake now builds a root `<Game>.exe` launcher and
        the real game executable under `Runtime/`; CopyRuntime places GEngine and
        vendor DLLs beside that executable. The publisher emits `Data/<Game>.gpak`
        and `Metadata/.gengine-output.json`, records category-relative paths,
        validates managed directory structure, and hard-links unchanged payloads.
        The launcher starts `Runtime/<Game>.exe`; `PreparePackagedResources`
        accepts the categorized `../Data/<Game>.gpak` location while retaining
        adjacent-pack compatibility. Nothing is embedded in the root EXE.
        Verification (2026-10-27): a fresh FrameGraphTriangle export produced a
        root with exactly one EXE and three folders (`Runtime`, `Data`,
        `Metadata`), with 20 Runtime files, one Data pack, and one Metadata
        manifest; launching the root EXE from `%TEMP%` completed 120 D3D12
        frames, all three GPU readbacks, and exit 0; a changed launcher update
        copied only one file; an unchanged repeat reported `copyFiles:0`; a
        missing-src failure preserved the existing manifest; and Qt Host
        `--packaging-self-test` passed build, layout, incremental repeat,
        failure preservation, cancel/retry, and close checks; full
        `GEngineTools.exe qt-editor --configuration Release --test` also passed
        configure/build, CTest, and real engine checks. Separate
        categorized packages built with GraphicsAPI 1/2/3 each completed 120
        frames, all three GPU readbacks, normal shutdown, and exit 0; logs are
        `%TEMP%/GEngineCategorized-{OpenGL,Vulkan,D3D12}.log`. Debug and
        clean-machine runs remain open for this layout change.

当前证据：ScriptEngine 使用显式 ScriptRuntimeRoot/ScriptCoreAssembly 路径，
仍加载固定 GEngine.Main 测试类；Mono 原生库静态链接，构建已切换到官方 SDK。
旧示例 BCL 无法追溯到匹配官方包，不可作为已验证 SDK；当前 Qt Worker 禁用脚本。
S2.1/S2.2 已验证显式清单部署和关闭脚本的三角形示例包；完整游戏/脚本包仍未完成。
