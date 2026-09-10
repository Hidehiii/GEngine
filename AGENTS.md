# GEngine engineering workflow

These instructions apply to work throughout this repository. Follow the user's
current request and preserve their existing changes. Communicate with the user
in Chinese unless they request another language. Write new code comments in
English only.

## Start with evidence

- Register every new feature, API, refactor, bug fix, example, tooling, or
  workflow/documentation task in the appropriate file under `TODO/` before
  implementation. Extend an existing item when it already covers the work;
  create a topic file when needed. Record scope and acceptance criteria, then
  update implementation progress, verification evidence, and remaining work.
  Keep completed entries as history; do not omit an item because it is small
  or completed within the same turn. Tracking an idea does not authorize work
  beyond the user's request.
- Read the relevant TODO entries, documentation, public headers, implementation,
  and example before proposing or changing an interface.
- Check Git status and relevant diffs first. Existing modifications belong to
  the user unless their origin is known. Preserve local backend selections and
  other unrelated edits; do not silently stage them.
- Distinguish an inspection request from authorization to implement. For an
  implementation request, carry the agreed work through code, verification,
  example, and documentation without asking the user to approve routine steps.
- Choose a bounded milestone with explicit acceptance criteria. Correctness and
  resource lifetime take priority over new abstractions and performance work.
- Treat previous build results and TODO checkboxes as evidence to recheck when
  affected, not proof that current code works on every backend.

## Architecture and implementation

- Keep engine headers under `GEngine/include` and implementation under
  `GEngine/src`, with corresponding directory structure.
- Keep application and renderer code backend-neutral. Native API handles,
  headers, synchronization, and allocation policies belong in backend code.
- Consider OpenGL, Vulkan, and D3D12 for every common rendering interface change.
  Implement supported semantics or report an explicit unsupported capability;
  do not silently turn required work into a no-op.
- Prefer device-owned resource creation and explicit queue submission. Preserve
  compatibility adapters where a migration is incomplete and document them.
- Avoid unrelated rewrites, vendor edits, or project-wide formatting. Use
  `apply_patch` for source and document edits. Preserve surrounding encoding and
  line-ending conventions.
- Update Premake sources when changing build configuration or file discovery;
  do not rely solely on edits to generated Visual Studio projects.
- Never put required work, validation with side effects, or error handling only
  inside an assertion. Release must execute the same required operations.

## GPU lifetime and synchronization rules

- Initialize every native handle to its API's null value, including optional
  allocations. Define who owns each handle and distinguish borrowed views from
  owning resources.
- Review construction, partial initialization, replacement, resize, and shutdown
  together. Destroy views/resources before their backing allocation, and device
  children before the device.
- CPU object destruction is not GPU completion. Retain resources through command
  recording and submission, and retire submitted resources only after all queues
  that may reference them have completed the relevant work.
- A reusable fence handle is not a permanent completion token. Associate
  completion with a submission generation or monotonic serial. Account for
  independent graphics, compute, and transfer work.
- Keep synchronization objects, command allocators, and retirement queues
  reclaimable. Check repeated-frame allocation behavior as well as shutdown.
- Device-wide waits are appropriate for shutdown and documented exceptional
  paths. Do not introduce per-frame waits as an unexplained lifetime fix.
- Resource descriptors must include the usages required by actual operations.
  A sampled texture is not automatically a render target or storage image.
- Check memory dependencies even when resource state does not change. Graph
  lifetime and dependency analysis must precede pooling or memory aliasing.

## Verification and examples

- For a substantial rendering feature or refactor, add or update an `Example`
  that actually exercises it. A triangle that never uses the new API does not
  validate that API.
- Document the example's launch/configuration, expected visible result, failure
  indicators, and relevant resize, replacement, and shutdown checks.
- For a bug, identify the concrete failing path and use a focused regression
  check or existing example that reproduces it. Avoid tests that merely repeat
  implementation details.
- Build the affected engine and example. Also check Release when changing
  assertion behavior, configuration, or code conditional on build mode.
- Run affected scenarios on available backends when feasible. Report each
  backend separately; never infer Vulkan or OpenGL success from D3D12, or vice
  versa. Include validation/debug output and memory behavior for lifetime work.
- Separate source inspection, build success, runtime checks, and user-reported
  results. Record untested cases and specific blockers honestly. Do not mark a
  milestone fully verified merely because it compiles.
- Documentation-only changes need link/content review, not an engine rebuild.

## Build discipline

- Before building, check for active compilers/builds and avoid concurrent writes
  to the same output directory or PCH. An open Visual Studio window alone does
  not prove a build is running.
- Own and track the build process until it finishes or is explicitly cancelled.
  Never terminate the user's build or leave an agent-started build unreported.
- Prefer concise build output and inspect the exit status. Do not rerun a known
  failing build repeatedly without changing the relevant condition.
- For the host's duplicate `PATH`/`Path` MSBuild error, use an isolated child
  environment and initialize the installed VS developer environment there.
  Do not alter global environment settings to work around it.
- Consult `Documentation/BuildAndRun.md` and current TODO notes for build issues.
  Recheck their status instead of treating historical blockers as permanent.

## Documentation, review, and handoff

- Update the relevant files in `Documentation`, example README, and
  `TODO/RenderingArchitecture.md` with implementation changes. Keep API examples
  consistent with the signatures and resource capabilities that actually exist.
- Keep incomplete TODOs open with a clear completed/remaining description. Add
  newly discovered blockers with a concrete reproduction or acceptance check.
- In the Codex app, open or refresh the unstaged review panel after material
  changes so the user can inspect added and removed lines. Include useful file
  links if the panel is unavailable.
- Give concise progress updates during sustained work. Explain findings and
  remaining uncertainty, rather than claiming success before verification.
- Before handoff, review the diff and run `git diff --check`. Report what changed,
  what was verified, and material remaining limitations.
- Commit when authorized by the user. Stage only intended files, inspect the
  staged diff, then verify the resulting commit and remaining worktree state.
  Report the commit hash. A request to commit does not imply a remote push.
- Do not claim that work will continue after the turn ends unless an authorized
  continuation mechanism has actually been set up.
