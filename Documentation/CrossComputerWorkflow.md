# Working on GEngine across computers

Use a separate topic branch for each computer or task. Commit source, Premake,
example and documentation changes together; leave generated projects, build
outputs, shader caches and machine-only backend choices out of shared commits.

Before integration, inspect `git status` on both computers. Save unfinished work
on a named backup branch; do not reset or replace a dirty working directory.
Fetch the other computer's pushed commits and merge in an isolated worktree.
Preserve both histories and review semantic overlap even when Git reports no
text conflict (for example, duplicate GPU allocation or initialization).

For rendering changes, regenerate projects from Premake and build Debug and
Release. Run FrameGraphTriangle separately on OpenGL, Vulkan and D3D12 with
`GENGINE_EXAMPLE_FRAME_LIMIT=120`. Verify the frame-119 compute readback and normal
shutdown, then exercise repeated replacement, memory behavior and resize as
appropriate. Record results and missing checks in TODO, rather than carrying a
parent branch's successful matrix forward as proof.

After the integrated commit is approved and pushed, the other computer should
fetch, inspect its local status, and fast-forward to the integrated history when
possible. If it has additional commits, merge those explicitly instead of using
a force push. Regenerate the local VS solution and rebuild before running;
generated projects and existing binaries do not update just because Git does.

The 2026-09-19 integration preserves local backup `8cc72a72` and remote parent
`66866fea`. Its decisions and verification are recorded in
[CrossComputerIntegration](../TODO/CrossComputerIntegration.md). Creating a
local integration commit does not by itself publish it to the other computer.
