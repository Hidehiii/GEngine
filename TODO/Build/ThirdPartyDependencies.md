  work, link remaining render-graph plan references to the rendering TODO, and
  require every TODO or planned task to be recorded in `TODO/`.
  Acceptance: no unregistered planned task remains in Documentation, each
  affected page links to its owning TODO, the Standard workflow states the
  mandatory rule, and `git diff --check` passes. This is a documentation-only
  change; no engine build is required.
  Implemented: build-system plans now live only in the workflow TODO and the
  pages link there; the live-log viewer is registered as optional future work;
  render-graph open scheduling/creation/pooling references link to the
  rendering TODO; and the Standard workflow explicitly requires TODO and
  planned tasks to be recorded in `TODO/`. Verified by link/content review and
  `git diff --check`.

## Remove the vendored Vulkan SDK snapshot (2026-10-29, implemented)

- [x] Remove `GEngine/GEngine-ThirdParty/VulkanSDK` from the shared dependency
  aggregate. Development machines must install a Vulkan SDK and export
  `VULKAN_SDK`; CMake and Premake generation fail with an actionable message
  when the environment variable is missing. Update engine/editor copying and
  build documentation so no script silently falls back to the removed snapshot.
  Acceptance: no main-repository or aggregate build script references the
  vendored path; a no-SDK generation fails before build-file output; a valid
  `VULKAN_SDK` path builds the engine/example; documentation states the
  mandatory prerequisite. Publishing the aggregate change and updating the main
  repository gitlink were required before claiming reproducible clean-machine
  behavior.
  Implemented: the aggregate commit removes all 646 tracked
  `VulkanSDK` files. CMake and Premake now require a non-empty `VULKAN_SDK`,
  validate the Vulkan header, loader import library, Debug/Release shaderc
  libraries and runtime DLLs, and fail generation when the SDK is missing or
  incomplete. Engine, editor worker, and project copying use the external SDK
  path. Build/API/architecture documentation and the root README state the new
  prerequisite; the README links to the official Windows Vulkan SDK download.
  Verification: with no `VULKAN_SDK`, both CMake VS2022 configuration and
  Premake VS2022 generation fail with actionable errors. With an external SDK
  path supplied in a child environment, CMake VS2022 configuration and a
  single-parallel Debug FrameGraphTriangle build passed, and Premake VS2022
  generation passed. The verification SDK was extracted only into ignored
  `build/verify-vulkan-sdk` from the aggregate's pre-change Git object for
  layout testing; this host still has no installed SDK.
  Published: aggregate commit
  `99c4b5e529f402e2292d0e367fabe5c356bbe995` removes the snapshot, and this
  main-repository change pins that commit. The development host now has
  Vulkan SDK 1.4.363.0 installed and its validation layer is visible; the
  shaderc-layout follow-up below must pass before the installed SDK is a
  complete build prerequisite. Clean-machine generation/build evidence remains
  a future environment check, not a claim of this item.
  Follow-up (2026-10-29): support the current official SDK layout. The installed
  Vulkan SDK 1.4.363.0 provides `Lib/shaderc_shared.lib` and
  `Bin/shaderc_shared.dll`, but not the old snapshot's
  `Lib/shaderc_sharedd.lib` or `Bin/shaderc_sharedd.dll`. It exposes VMA
  through the optional `com.lunarg.vulkan.vma` package.
  Implemented: Debug and Release now link the official
  `Lib/shaderc_shared.lib`; generation and runtime copying no longer require
  the old Debug shaderc names. The official VMA package is installed on this
  host, the repository-owned `vk_mem_alloc.h` is removed, and engine includes
  use the SDK's `vma/vk_mem_alloc.h` path. Both generators require the official
  Vulkan and VMA headers.
  Verification: with `VULKAN_SDK=C:\VulkanSDK\1.4.363.0`, CMake VS2022
  configuration and Premake VS2022 generation passed. Single-parallel CMake
  Debug and Release FrameGraphTriangle builds passed with existing warnings.
  A source scan found no remaining `shaderc_sharedd`, local VMA include, or
  vendored VulkanSDK build references. Clean-machine generation/build evidence
  remains future environment work.

## Third-party dependency layout and submodules (implemented)

### Editor compatibility repair (2026-10-23, in progress)

- [ ] Repair reported vertically flipped editor presentation (2026-10-24).
  Scope: use GUIUtils attachment UV endpoints for the final fullscreen quad;
  preserve scene image UVs and backend configuration. Acceptance: quad top
  samples texture top on OpenGL/Vulkan/D3D12, Debug/Release builds and bounded
  backend runs pass; record visual verification separately.
  Implemented: final quad top/bottom UVs now come from GUIUtils, preserving
  OpenGL orientation and correcting Vulkan/D3D12 attachment presentation.
  Verified: CMake VS2022 Debug/Release builds and all six 120-frame backend
  runs exited 0 (tmp/editor-layout-*/output.log); no captured validation errors.
  Source review confirms top V=1 for OpenGL and V=0 for Vulkan/D3D12.
  Remaining: user/visual confirmation of upright panels and gizmo alignment.

- [ ] Migrate editor build and rendering/input/camera calls to current engine
  APIs without disabling scene editing. Acceptance: Debug/Release editor builds,
  bounded startup/render/resize/shutdown checks on supported backends, explicit
  unsupported capabilities, and synchronized API/architecture documentation.
  Preserve backend configuration and unrelated local files. Initial failures:
  obsolete layer hooks, material/vertex/framebuffer interfaces, camera MSAA,
  Renderer2D overlays, and input names. Record runtime blockers separately.
  Preview runs exposed invalid OpenGL sampler magnification and combined-texture
  binding enums. Correct these calls and recheck textured presentation without
  GL errors. Continue Debug/Release backend verification after interruption.
  Vulkan preview exposed destruction of draw buffers/pipelines while recording:
  VulkanCommandBuffer::Render did not retain its pipeline (unlike D3D12). Retain
  graphics/compute pipelines through recording/submission and verify retirement.
  Combined image samplers were absent from SPIR-V reflection, yielding an empty
  presentation descriptor layout. Reflect sampled_images and validate the editor
  texture-present shader using a fresh shader cache on all backends.
  Implemented: portable preview command recording and UI-target presentation,
  current input/config/event APIs, hierarchy selection, default docking layout,
  optional managed runtime detection, and a working-directory-aware launcher.
  Preview draws image colors and indexed mesh vertex colors; textured/lit/custom
  materials and camera MSAA are explicitly unsupported, not silently ignored.
  D3D12 UI descriptors retain textures per reusable completed frame slot; Vulkan
  UI descriptors are recycled per frame and freed before pool shutdown. Vulkan
  command buffers retain graphics/compute pipelines. Scene entity iteration and
  combined-image-sampler reflection were repaired along with OpenGL native enums.
  Verified 2026-10-24: CMake VS2022 Debug/Release engine/editor builds passed;
  six fresh-layout 120-frame editor runs (D3D12/OpenGL/Vulkan in both configs)
  exited 0 with no captured validation/GL/assertion errors. Release launcher run
  from the real editor directory also reached frame 120 and exited 0, with the
  existing managed runtime available. Premake VS2022 generation passed and its
  editor debugger working directory is now set. git diff --check passed.
  Evidence: tmp/editor-layout-*/output.log and tmp/editor-launcher-Release.log.
  Remaining: full Premake builds, visual/gizmo/camera and scene save/load/play
  interaction checks, real resize/minimize/restore, and measured long-run memory.
  Computer-use window inspection timed out awaiting app access; do not count it
  as a visual pass. The unbounded diagnostic process was explicitly stopped.
  Final regression: FrameGraphTriangle Debug/Release builds and six 120-frame
  D3D12/OpenGL/Vulkan runs also exited 0 after the common rendering fixes.
- [x] Rebuild the current Qt editor after the legacy third-party output
      cleanup (2026-10-27). Scope: run the standard Release CMake editor build,
      including the missing Qt 6.10.3 Release SDK prerequisite, Qt Host, and
      GEngine-EditorEngineWorker, without changing engine API, architecture,
      backend selection, or source. Acceptance: BuildQt6.ps1 and
      BuildQtEditor.ps1 exit successfully, their configure/build/install logs
      contain no command failure, and updated SDK/Host/Worker artifacts appear
      in their unified dist paths. Runtime interaction is outside this rebuild
      request unless the build output requires a focused check.
      Implemented: built and installed the Qt 6.10.3 Release SDK, then rebuilt
      the Qt Host and GEngine-EditorEngineWorker through the standard CMake
      editor script. No source, build configuration, engine API, architecture
      surface, or backend selection changed.
      Verification (2026-10-27): `BuildQt6.ps1 -Configuration Release` configure,
      build, and install commands each exited 0. `BuildQtEditor.ps1
      -Configuration Release` Qt configure/build and engine configure/build
      commands each exited 0. Updated artifacts exist under
      `dist/qt6/windows-x86_64/Release`, `dist/qt-editor/windows-x86_64/Release/
      GEngine-EditorHost`, and `dist/qt-editor/windows-x86_64/Release/
      GEngine-EditorEngineWorker`. An offscreen Host self-test with engine-check
      and the rebuilt Worker exited 0 and reported the recovery suite PASS.
      Manual visual/interaction checks were not run and remain separate.

### In progress: ImGui, Box2D, and SPIRV-Cross upgrade (2026-10-22)

- [ ] Upgrade ImGui to pinned v1.93.0-docking, replace Box2D with upstream
  v2.4.1, and build pinned upstream SPIRV-Cross core/GLSL from the same source
  tree as its headers. Preserve existing documentation edits. Move project
  overlays into first-party build scripts and use upstream repository names.
  Adapt engine UI backends, texture/font handling, and incompatible ImGuizmo
  calls without silently disabling required functionality. Acceptance: both
  Premake and CMake generate; Debug/Release engine, editor, and affected example
  builds pass; available backend runs exercise shader reflection/conversion and
  UI behavior. Record exact pins, checks, and blockers before marking complete.
  Commit and push authorized on 2026-10-22: publish the aggregate first, then
  commit its fixed gitlink with the engine adaptations in the parent repository.
  UI smoke exposed a startup crash: backend attach accesses Application runtime
  before its constructor assigns ownership. Defer UI initialization until that
  assignment; verify startup and shutdown separately on each backend.
  Implemented locally: imgui v1.93.0-docking at
  b916186e66aa3c02b72c6ca17455516512ed8049; box2d v2.4.1 at
  9ebbbcd960ad424e03e5de6e66a40764c16f51bc; SPIRV-Cross at
  300681bbcb8365a8d1eacdacdbdd0249bbf1d6b1. Upstream trees are unmodified.
  Premake overlays moved to BuildScripts/Premake; SPIRV-Cross core/GLSL are
  source-built. Adapted ImGui backend initialization and image-button IDs,
  font-size access, and uint64_t GUI texture IDs. A first-party translation unit
  adapts legacy ImGuizmo capture/polyline calls; the Gizmo facade stays planned.
  UI smoke also exposed and fixed nonvirtual PlatformImGui destruction, shutdown
  access to an already-cleared Application runtime, missing D3D12 UI command
  buffers, and a command-list object cast incorrectly as an array. D3D12 UI now
  uses tracked submission rather than bypassing waits/signals/resource retention.
  Verified: Premake and CMake VS2022 generation; CMake engine and
  FrameGraphTriangle Debug/Release builds. Twelve 120-frame runs (graph and UI,
  Debug/Release, D3D12/OpenGL/Vulkan) each exited 0 and passed all three graph
  readbacks; captured logs contained no error/validation/assertion messages.
  Evidence: parent workspace tmp/dependency-build-{Debug,Release}.log and
  tmp/upgrade-*/output.log. FBX LFS libraries were restored after an initial
  pointer-file linker failure. Full editor builds remain blocked by existing
  OnDetach override, Material::Create, VertexBuffer::SetLayout/SetData, old
  framebuffer/Renderer2D, and Input API usage; see editor compile logs in tmp.
  Remaining: editor migration and visual interaction checks, Box2D physics runtime
  checks, long-run memory/real resize checks, full Premake engine builds, fresh
  recursive-clone verification. Aggregate e05958d was committed and pushed;
  this parent change records that fixed dependency revision.
  Existing backend selection is preserved. Publication is handled aggregate-first;
  the parent gitlink records the dependency commit used by this implementation.

### Planned: engine-owned Gizmo forwarding interface

2026-10-24 superseding implementation scope (user authorized): convert ImGuizmo
to a pinned upstream submodule now. Compile it in the editor executable behind
an editor-owned GizmoController, remove engine public-header/export dependencies
and the legacy compatibility translation unit. This replaces the engine-DLL
forwarding proposal below, consistent with future EditorHost ownership. Preserve
the current shared ImGui context and call BeginFrame once per editor UI frame.
Acceptance: pristine upstream tree; CMake/Premake generation and Debug/Release
engine/editor builds; bounded OpenGL/Vulkan/D3D12 runs; document exact pin,
remaining interactive checks, and migration boundaries. Do not commit/push the
parent or aggregate without separate authorization in this task.
Publication authorized by the user on 2026-10-24: commit/push the aggregate
first, then record its exact gitlink with the editor integration and docs in
the parent. Exclude local Config.ini, imgui.ini and temporary verification files.

- [x] Local conversion: added upstream ImGuizmo at
  `105e4fde45e4e74aae8973d98750820ef7aca7e2` (v1.92.5 WIP header) under the
  aggregate's `.gitmodules`. Replaced the modified v1.61 snapshot; upstream tree
  is clean, MIT license retained, Git connectivity checked with no temporary
  object-store dependency. Aggregate commit
  `b512654cd9b05282d54828fbb16f315659a41074` records the conversion; the parent
  gitlink is updated to that fixed revision for publication. Aggregate pushed
  to origin/master on 2026-10-24 after retrying a transient GitHub TLS error.
- [x] Editor-only integration: GizmoController maps operations/spaces explicitly,
  binds the current context, sets the viewport draw list/rectangle and forwards
  manipulation/grid drawing. Engine public include and BeginFrame coupling plus
  ImGuizmoCompatibility.cpp are removed. Both generators compile only upstream
  src/ImGuizmo.cpp in the editor using the project's existing ImGui.
- [ ] Complete validation: CMake/Premake generation passed; CMake Debug engine
  and editor builds passed, and three Debug 120-frame backend runs exited 0.
  Focused headless ImGui/controller check emitted gizmo geometry for all three
  operations in local/world space; synthetic translation drag and 0.5-unit
  snapping passed (tmp/gizmo-smoke-runtime.log). Debug DLL export inspection
  confirms no ImGuizmo symbols. Release engine/editor build and three Release
  120-frame backend runs also exited 0; Release DLL has no ImGuizmo exports.
  No captured validation/GL/assertion errors in the six final editor logs.
  Evidence: tmp/dependency-build-{Debug,Release}.log, tmp/editor-layout-*/output.log
  and tmp/gizmo-smoke-runtime.log. Both repository diff checks and document link
  review passed. Existing compiler warnings remain; full Premake builds and a
  fresh recursive clone after aggregate publication have not been verified.
  Manual rotation/scale/snapping/hover/capture, resize and long-run resource
  checks remain open; bounded runs alone do not verify these interactions.

Historical proposal (superseded; do not implement as written):
Add an exported GEngine Gizmo interface that forwards to ImGuizmo internally,
  and migrate editor calls to it before converting ImGuizmo to an upstream submodule.
  Scope: engine-owned operation/space enums, viewport rectangle, camera and transform
  matrices, snapping parameters, and hover/active results; public declarations must
  not expose ImGuizmo enums or ImDrawList pointers. Map operations explicitly rather
  than casting editor combo-box indices. Keep ImGuizmo implementation inside the
  engine DLL, export only the engine wrapper, and remove the third-party header's
  GENGINE_API/Core.h dependency when adopting the pinned upstream tree. Preserve
  BeginFrame timing, the active ImGui context, and viewport draw-list selection.
  Update Premake and CMake integration; keep graphics backend details internal.
  Acceptance: the editor uses only the engine Gizmo interface for gizmo operations;
  upstream ImGuizmo requires no engine-specific source patch; Debug/Release engine
  and editor builds pass; editor checks cover translation, rotation, scaling,
  snapping, local/world space, hover/active state, viewport resize, and shutdown.
  Check D3D12, Vulkan, and OpenGL separately where available and record blockers.
  Historical status: planned only; replaced by the editor-owned scope above.
  Documentation registration verified by content/link review and git diff --check;
  implementation, builds, and editor runtime verification remain open.

- [x] Rename `GEngine/vendor/` to `GEngine/Third-Party/` and make the resulting
  dependency tree a pinned Git submodule. Before implementation, identify or
  create the intended remote repository for that aggregate submodule and
  inventory each dependency's version, local patches, license, binary assets,
  and consumers. Preserve the nested `SPIRV-Reflect` submodule and its pinned
  commit in the new repository. Update `.gitmodules`, Premake, CMake, source
  includes, scripts, documentation, and
  ignore rules. Do not replace SDKs or binary-only packages with guessed
  upstream revisions or discard them to fit the new repository. Acceptance:
  the chosen submodule remote and pinned revision are documented; a fresh
  clone with recursive submodule initialization reproduces required files;
  Premake and CMake project generation resolve the new paths; Debug and Release
  GEngine plus FrameGraphTriangle builds and bounded backend runs pass; no
  local vendor patch or required runtime DLL is silently lost. Record the
  dependency provenance, verification evidence, and remaining work here when
  implementation is authorized.
  Initial inspection: `GEngine/vendor/` currently contains 22 top-level
  dependency directories (the earlier count of 23 is stale) and about 8,353
  tracked paths; `SPIRV-Reflect` is already a submodule at commit
  `74cd6a01f274d6cc7fc9f14c9e3608047b09c5af`. Current Premake and CMake
  definitions reference `GEngine/vendor/` directly. The aggregate submodule
  remote has not been specified. Status: plan only; no dependency move or
  submodule conversion has been made.

  Upstream investigation (2026-10-20): candidate repository URLs were checked
  with `git ls-remote`; no upstream repository was downloaded. The following
  dependencies are the best candidates for individual Git submodules:
  `DirectX-Headers` from
  `https://github.com/microsoft/DirectX-Headers.git` (`v1.615.0`),
  `glm` from `https://github.com/g-truc/glm.git` (`1.0.0`), and `json`
  (JsonCpp) from `https://github.com/open-source-parsers/jsoncpp.git`
  (`1.9.6`). These are close to complete source snapshots and have clean
  version tags, so they are the lowest-risk first group.

  A second group can use submodules only after preserving GEngine integration
  as an overlay or fork: `box2d` from
  `https://github.com/erincatto/box2d.git` (`v2.4.1`), `GLFW` from
  `https://github.com/glfw/glfw.git` (`3.4`), `ImGui` from
  `https://github.com/ocornut/imgui.git` (local snapshot says `1.89.3 WIP`),
  and `yaml` (yaml-cpp) from `https://github.com/jbeder/yaml-cpp.git`
  (`yaml-cpp-0.7.0`). Their current trees contain GEngine Premake/Visual
  Studio project files; ImGui also contains local source-level changes and
  must not be assumed identical to upstream tag `v1.89.3`.

  A third group needs layout cleanup or exact-commit identification before
  conversion: `entt` from `https://github.com/skypjack/entt.git`
  (`v3.13.0`, currently an amalgamated header), `spdlog` from
  `https://github.com/gabime/spdlog.git` (`v1.13.0`, duplicated under
  `spdlog-1.x`), `HLSLcc` from
  `https://github.com/Unity-Technologies/HLSLcc.git`, `ImGuizmo` from
  `https://github.com/CedricGuillemet/ImGuizmo.git`, `SPIRVCross` from
  `https://github.com/KhronosGroup/SPIRV-Cross.git`, and `stb` from
  `https://github.com/nothings/stb.git`. The latter four do not expose a
  single local version tag and require tree comparison against upstream to
  pin the exact commit. `SPIRV-Reflect` is already a submodule and should be
  preserved as-is.

  Implemented (2026-10-21): the aggregate remote is
  `https://github.com/Hidehiii/GEngine-ThirdParty.git`, pinned by the main
  repository at `GEngine/GEngine-ThirdParty` commit
  `4efd7cce435fb8a2ee173f17fb0e4c2b9ffe670e`. The aggregate repository records
  provenance, local overlays, binary assets, consumers, and license gaps in
  `INVENTORY.md`; preserves `SPIRV-Reflect` at
  `74cd6a01f274d6cc7fc9f14c9e3608047b09c5af`; and tracks the four files over
  100 MiB with Git LFS. Premake, CMake, example projects, source includes,
  runtime DLL copy paths, tests, ignore rules, and documentation now resolve
  `GEngine/GEngine-ThirdParty`. The root repository no longer tracks the old vendor
  tree directly.

  The first aggregate snapshot commit was
  `ef1b0a11b13c9d6bb23a2da9e96650faa554a1a5`. A tracked-tree comparison found
  that it had accidentally included 209 local generated build outputs under
  `box2d`, `GLAD`, and `GLFW` and omitted the source repository's tracked but
  ignored `box2d/.idea/workspace.xml`. Correction commit
  `4efd7cce435fb8a2ee173f17fb0e4c2b9ffe670e` removed those generated outputs,
  restored the workspace file, and added explicit ignore rules. The final
  aggregate has 8,359 tracked paths: all 8,353 originally tracked vendor paths
  plus `.gitattributes`, `.gitignore`, `.gitmodules`, `INVENTORY.md`, `LICENSE`,
  and `README.md`; the comparison reports no missing original tracked path.
  The first Premake build exposed an incorrect `../Third-Party` project-relative
  source path for `ImGuizmo`; `GEngine/premake5.lua` now uses the correct
  in-project aggregate paths. The aggregate directory was subsequently renamed
  from `Third-Party` to `GEngine-ThirdParty` to match its remote repository name.

  Verified (2026-10-21): a fresh `git clone --recurse-submodules` with Git LFS
  reproduced the final aggregate at 8,359 paths, checked out `SPIRV-Reflect`
  and its nested googletest at the pinned commits, passed `git lfs fsck`,
  contained zero tracked generated outputs, and matched the original SHA-256
  hashes for all four LFS-tracked binaries. Premake VS2022 generation and
  Debug/Release direct project builds of FrameGraphTriangle, including its
  GEngine dependency, passed. CMake VS2022 configuration and Debug/Release
  builds of GEngine plus FrameGraphTriangle passed in an isolated developer
  environment after reproducing the known host duplicate `PATH/Path` MSBuild
  issue. Six bounded 120-frame CMake FrameGraphTriangle runs passed separately
  for D3D12, OpenGL, and Vulkan in Debug and Release; each exited 0 and passed
  compute, compute-first, and texture-array readbacks at frame 119. Premake
  D3D12 Debug/Release 120-frame runs also passed. The example configuration
  was restored to D3D12 after the runs.

  Remaining: exact upstream tree comparisons remain unverified for snapshots
  marked as such in the aggregate inventory, and redistribution terms still
  require confirmation for binary packages without local license/notice files.
  Those provenance refinements are tracked below and do not alter the pinned
  aggregate content.

- [ ] Refine provenance for unverified dependency snapshots. Compare local
  source trees against their candidate upstream commits, record the exact
  commit or preserve the GEngine overlay/fork reason, and confirm/obtain
  redistribution terms for `FBXSDK`, `GLAD`, `OpenSSL`, `PhysX`, `shaderc`, and
  `VulkanSDK` where the current snapshot lacks a license or notice file.
  Acceptance: the aggregate `INVENTORY.md` has no unresolved exact-commit or
  license-gap entry, and any added license/notice files are tracked without
  changing dependency binaries.

- [~] Convert qualified source dependencies inside `GEngine/GEngine-ThirdParty` into
  individual Git submodules. A dependency is qualified only after proving that
  its current tracked tree maps to an exact upstream commit, or after moving
  GEngine-specific project files/patches to a pinned fork or explicit overlay
  mechanism so no local change is silently lost. Binary-only SDK packages stay
  tracked as aggregate snapshot content unless their provider supplies a
  reproducible pinned remote. Acceptance: each converted dependency has an
  aggregate-local `.gitmodules` entry and gitlink whose directory and submodule
  name equal the remote repository name, the main repository updates
  the aggregate submodule pin, a fresh recursive Git LFS clone reproduces all
  dependency files, and Debug/Release GEngine plus FrameGraphTriangle builds
  and bounded backend runs still pass.
  Qualified and converted (2026-10-21): `DirectX-Headers` is now an
  aggregate-local submodule pinned at `27d63115c1037d0e4e119085f34345bd9ecf7ada`
  (`v1.615.0`); JsonCpp (`jsoncpp`) is pinned at
  `89e2973c754a9c02a49974d839779b151e95afd6` (`1.9.6`); `HLSLcc` is
  pinned at `3ea1fcd6bd0ac445bc078de0bf32f0950188577b` after an exact tracked-tree
  match; `entt` is pinned at
  `fedcb920ce0068c35ffbc66fd4e84864e6ef71ef` (`v3.13.0`). Every local
  tracked file was compared against the target tag. `DirectX-Headers` differed
  only by line endings, and `json` differed only by the executable mode of
  `amalgamate.py`; both now use the authoritative upstream tree. `HLSLcc`
  matched its upstream tree exactly. `entt`'s local `include/entt.hpp` and
  `LICENSE.txt` matched upstream `single_include/entt/entt.hpp` and `LICENSE`
  exactly, so its include path now uses the upstream layout.
  The aggregate commit containing these submodules is
  aggregate commit `91ffb9e797f2e0d8e34571cef4dda14cbcc6e29a`, and the main repository pins it through
  `GEngine/GEngine-ThirdParty`.
  Verified: a fresh recursive Git LFS clone reproduced the aggregate at 7,971
  tracked paths and checked out `DirectX-Headers`, `jsoncpp`, `HLSLcc`, `entt`,
  `SPIRV-Reflect`, and nested googletest at their pinned commits; `git lfs fsck`
  passed. CMake VS2022
  Debug/Release builds of GEngine and FrameGraphTriangle passed, and 120-frame
  D3D12 runs in both configurations exited 0 with all three graph readbacks.
  Premake VS2022 Debug/Release builds and D3D12 runs likewise passed in both
  configurations. Submodule directories are required to use the remote
  repository name: the engine-level aggregate is `GEngine-ThirdParty`, and the
  JsonCpp aggregate-local submodule is `jsoncpp`.
  Qualification audit for the remaining source snapshots: `glm` is a smaller,
  different `1.0.0` snapshot; `box2d` contains a local `SetUserData` addition,
  an updated doctest tree, and project overlays; `GLFW` has 82 modified common
  files plus overlays; `ImGui` has 55 modified files and source-level changes;
  `yaml` is a subset with 33 modified files and overlays; `spdlog` has local
  patches in both its main and duplicate trees; `ImGuizmo` has 21 modified files
  against tag `1.83`; `SPIRVCross` has 35 modified files and 26 missing files
  against the closest API-0.59 commit; and `stb` mixes files from different
  upstream dates. None are safe drop-in submodule replacements without a pinned
  fork or explicit overlay mechanism. New submodule directories must use the
  remote repository name exactly; if a historic local directory name differs,
  rename it in the same change that introduces the submodule.
  Remaining: after exact provenance comparisons, qualify or explicitly reject
  the other source snapshots; binary-only packages remain aggregate snapshot
  content unless a reproducible pinned remote exists.
  Implemented target upgrade set (2026-10-21): `glm` is pinned at
  `8d1fd52e5ab5590e2c81768ace50c72bae28f2ed` (`1.0.3`); `glfw` at
  `d9d6f0f1f967807ffade6598ea9a631ebaf37a56` (`3.5.1`); `yaml-cpp` at
  `56e3bb550c91fd7005566f19c079cb7a503223cf` (`yaml-cpp-0.9.0`); `spdlog` at
  `79524ddd08a4ec981b7fea76afd08ee05f83755d` (`v1.17.0`); and `stb` at
  `a7d4c2175630e1ea5b6f8dd00ab96fed0b97b96d`. Their aggregate-local directories
  use the remote repository names.
  Verified after the upgrade: Premake and CMake VS2022 Debug/Release builds of
  GEngine plus FrameGraphTriangle passed. Six bounded 120-frame runs on D3D12,
  OpenGL, and Vulkan in both configurations exited 0 and passed all three graph
  readbacks. D3D12 Debug Layer emitted two cross-queue UAV-barrier errors; that
  diagnostic is tracked separately and does not alter the dependency pin.
  A fresh recursive Git LFS clone of aggregate commit
  `91ffb9e797f2e0d8e34571cef4dda14cbcc6e29a` reproduced 6,845 tracked paths,
  checked out all aggregate-local submodules at their pinned commits, passed
  `git lfs fsck`, and matched the four expected LFS SHA-256 object hashes.
