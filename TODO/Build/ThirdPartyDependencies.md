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

## Third-party dependency layout and submodules (planned)

- [ ] Rename `GEngine/vendor/` to `GEngine/Third-Party/` and make the resulting
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
