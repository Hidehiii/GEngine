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

## Third-party dependency layout and submodules (implemented)

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
