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
  repository at `GEngine/Third-Party` commit
  `4efd7cce435fb8a2ee173f17fb0e4c2b9ffe670e`. The aggregate repository records
  provenance, local overlays, binary assets, consumers, and license gaps in
  `INVENTORY.md`; preserves `SPIRV-Reflect` at
  `74cd6a01f274d6cc7fc9f14c9e3608047b09c5af`; and tracks the four files over
  100 MiB with Git LFS. Premake, CMake, example projects, source includes,
  runtime DLL copy paths, tests, ignore rules, and documentation now resolve
  `GEngine/Third-Party`. The root repository no longer tracks the old vendor
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
  in-project `Third-Party/...` paths.

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

- [~] Convert qualified source dependencies inside `GEngine/Third-Party` into
  individual Git submodules. A dependency is qualified only after proving that
  its current tracked tree maps to an exact upstream commit, or after moving
  GEngine-specific project files/patches to a pinned fork or explicit overlay
  mechanism so no local change is silently lost. Binary-only SDK packages stay
  tracked as aggregate snapshot content unless their provider supplies a
  reproducible pinned remote. Acceptance: each converted dependency has an
  aggregate-local `.gitmodules` entry and gitlink, the main repository updates
  the aggregate submodule pin, a fresh recursive Git LFS clone reproduces all
  dependency files, and Debug/Release GEngine plus FrameGraphTriangle builds
  and bounded backend runs still pass.
  Qualified and converted (2026-10-21): `DirectX-Headers` is now an
  aggregate-local submodule pinned at `27d63115c1037d0e4e119085f34345bd9ecf7ada`
  (`v1.615.0`); `json` is pinned at
  `89e2973c754a9c02a49974d839779b151e95afd6` (JsonCpp `1.9.6`). Every local
  tracked file was compared against the target tag. `DirectX-Headers` differed
  only by line endings, and `json` differed only by the executable mode of
  `amalgamate.py`; both now use the authoritative upstream tree. `glm` was not
  converted because its local tree is a substantially smaller, different
  snapshot of upstream `1.0.0` rather than an exact match or overlay.
  The aggregate commit containing these submodules is
  `8942f70419de3981ad26ac3b747860e00da2b9be`, and the main repository pins it
  through `GEngine/Third-Party`.
  Verified: a fresh recursive Git LFS clone reproduced the aggregate at 8,037
  tracked paths and checked out `DirectX-Headers`, `json`, `SPIRV-Reflect`, and
  nested googletest at their pinned commits; `git lfs fsck` passed. CMake VS2022
  Debug/Release builds of GEngine and FrameGraphTriangle passed, and 120-frame
  D3D12 runs in both configurations exited 0 with all three graph readbacks.
  Premake VS2022 Debug/Release builds and D3D12 runs likewise passed in both
  configurations.
  Remaining: after exact provenance comparisons, qualify or explicitly reject
  the other source snapshots; binary-only packages remain aggregate snapshot
  content unless a reproducible pinned remote exists.
