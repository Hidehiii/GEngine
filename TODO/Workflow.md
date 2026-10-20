# Workflow tracking

## Documentation and TODO topic splitting (2026-10-20)

- [x] Split oversized TODO and rendering documents into focused topic files
  while preserving all history and evidence. Scope: move the large rendering,
  workflow, integration, render-graph, graphics-API, and rendering-architecture
  sections into topic-specific files; keep the original files as concise indexes
  so existing links continue to resolve; update all affected cross-links. No
  code or public API changes are included. Acceptance: no documentation file
  exceeds roughly 200 lines except where a single coherent topic requires it;
  all moved content is reachable from its index; Markdown links resolve; and
  `git diff --check` passes. Documentation-only change; no engine build is
  required.
  Implemented: split the workflow, integration, and rendering TODO histories
  into `TODO/Build/`, `TODO/Integration/`, and `TODO/Rendering/`; split the
  render-graph, graphics-API, and rendering-architecture guides into
  `Documentation/API/RenderGraph/`, `Documentation/API/Graphics/`, and
  `Documentation/Architecture/Rendering/`; and retained the original files as
  concise indexes so existing links continue to resolve.
  Verified: the largest Markdown file is now 194 lines, all repository Markdown
  links resolve, and `git diff --check` passes. No engine build was run because
  this is a documentation-only restructuring.

## Documentation plan tracking (2026-10-20)

- [x] Keep documentation implementation-only and move plan ownership into
  `TODO/`. Scope: replace the detailed planned-build paragraphs in the
  architecture/API build pages with links to their existing workflow TODO
  entries, register the optional live-log viewer as runtime-logging future
  work, link remaining render-graph plan references to the rendering TODO, and
  require every TODO or planned task to be recorded in `TODO/`.
  Acceptance: no unregistered planned task remains in Documentation, each
  affected page links to its owning TODO, the Standard workflow states the
  mandatory rule, and `git diff --check` passes. This is a documentation-only
  change; no engine build is required.

## Topic index

- [Third-party dependencies](Build/ThirdPartyDependencies.md)
- [Build output paths](Build/BuildOutputPaths.md)
- [Toolchain and Visual Studio compatibility](Build/Toolchain.md)
