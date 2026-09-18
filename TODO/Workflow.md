# Workflow tracking

- [x] Establish repository engineering standards in `AGENTS.md` and link them
  from the documentation index.
- [x] Require every new work item to be registered in `TODO/` before implementation,
  then retain its scope, acceptance criteria, implementation and verification
  status through completion. Acceptance: the rule is documented in `AGENTS.md`.
  Verified by reviewing the written rule and this tracking entry.

## Visual Studio 2026 project generation

- [ ] Add a VS2026 option to `Scripts/GenerateProjects.bat`, preserving the
  existing VS2022/VS2019 choices. Allow an external Premake executable without
  replacing vendored binaries, and resolve the repository relative to the script.
  Acceptance: `vs2026` is dispatched explicitly; unsupported Premake versions
  fail with actionable guidance; generation failures return a nonzero status;
  usage documentation names the real script and explains the prerequisite.
  Evidence before implementation: the bundled Premake reports `5.0.0-beta2`;
  its `--help` lists `vs2022` and `vs2019`, but not `vs2026`.
  Implemented: menu option `3`, noninteractive `vs2026` argument, `PREMAKE_EXE`
  override, script-relative repository resolution, explicit unsupported-action
  diagnostics and exit-code propagation. Updated build instructions and fixed
  the stale `GenerateProject.bat` links.
  Verified: bundled-Premake VS2026 rejection, invalid argument and missing
  executable each returned exit code 1. Reviewed the diff and documentation
  targets; `git diff --check` passed.
  Initial validation limitation: VS2026 generation required an external
  Premake (resolved by the bundled upgrade below).
  VS2019/VS2022 regeneration was skipped after detecting MSBuild
  PID 29284 on 2026-09-18, to avoid modifying projects during another build.
  Engine/example Debug and Release builds and backend runtime checks were not
  run; no engine, rendering API, example, or backend configuration was changed.

### Bundled Premake upgrade (user-authorized follow-up)

- [x] Download the official Premake 5.0.0-beta8 Windows release and replace
  `vender/bin/premake/premake5.exe`. Acceptance: verify the release asset
  checksum when provided, executable version and `vs2026` action; update usage
  documentation so VS2026 no longer requires an external executable.
  Before implementation: MSBuild PID 29284 and cl PID 19944 were active on
  2026-09-18. Do not regenerate shared project files while that build runs.
  Implemented: replaced only the executable from the official
  `premake/premake-core` release `v5.0.0-beta8`, asset
  `premake-5.0.0-beta8-windows.zip`; updated script diagnostics and usage docs.
  Verified: ZIP SHA256 matches the official GitHub release asset page:
  `e64ce2ed8778e0098f63674cca61fe33941b5f0c8d9a4afd651152bdea3758ab`.
  Installed executable matches the extracted release, SHA256:
  `2301e3e23ff3074cb83a5ea6103d68c7ea81dad56b786807c84b0643cddea31b`.
  `--version` reports `5.0.0-beta8`; `--help` lists `vs2019`, `vs2022` and
  `vs2026`. Documentation and diff review completed; `git diff --check` passed.
  Full engine/example generation and Debug/Release builds remain unverified:
  MSBuild PID 29284 and an active compiler were still running during validation,
  so shared generated project files were left untouched. Existing `flags`
  deprecation warnings appear under beta8; unrelated vendor scripts were not
  changed. The binary replacement milestone is complete, not full build
  compatibility verification.
