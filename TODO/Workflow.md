# Workflow tracking

## Documentation and TODO topic splitting (2026-10-20)

- [x] Restructure the README-to-documentation index hierarchy. Scope: make the
      root README link only to the project documentation entry document, and
      make `Documentation/README.md` index every current Documentation page plus
      project-level README, example, tests, and engineering-workflow entries.
      Add a root README TODO entry and make this workflow file's topic index
      cover every current TODO topic. Acceptance: the hierarchy is root README →
      documentation entry → individual documents, root README also reaches the
      TODO index, every current Documentation and TODO file is reachable, all
      relative Markdown links resolve, and `git diff --check` passes.
      Documentation-only change; no engine build is required.
      Implemented: the root README now links directly to the project
      documentation entry and the TODO workflow index. `Documentation/README.md`
      indexes all 42 other Documentation pages plus project-level editor,
      example, test, and engineering-workflow references. `TODO/Workflow.md`
      now indexes all 25 other TODO topic files. The root README no longer
      duplicates individual page links.
      Verification (2026-10-27): automated coverage checks confirmed the
      Documentation entry covers every other Documentation Markdown file, the
      TODO topic index covers every other TODO Markdown file, the root README
      reaches both entries, all index links resolve, and `git diff --check`
      passes. No engine build was run for this documentation-only change.

- [x] Update the root README documentation index to link every documentation
      area. Scope: add Architecture, API, Guides, Workflow, Diagnostics,
      editor, example, tests, and engineering-workflow entry points; keep the
      root README as an index rather than duplicating detailed content.
      Acceptance: all linked files exist, the index covers every current
      top-level documentation area, relative Markdown links resolve, and
      `git diff --check` passes. Documentation-only change; no engine build is
      required.
      Implemented: the root README now links the documentation home and every
      top-level Architecture, API, Guides, Workflow, Diagnostics, editor,
      example, tests, and engineering-workflow entry point. It remains an index
      rather than duplicating detailed page content.
      Verification (2026-10-27): all README Markdown links resolve, every
      intended documentation area is present, and `git diff --check` passes.
      No engine build was run for this documentation-only change.

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

### Build

- [Third-party dependencies](Build/ThirdPartyDependencies.md)
- [Build output paths](Build/BuildOutputPaths.md)
- [Toolchain and Visual Studio compatibility](Build/Toolchain.md)

### Rendering

- [Rendering architecture overview](RenderingArchitecture.md)
- [Rendering architecture phases](Rendering/ArchitecturePhases.md)
- [Rendering architecture phase 1](Rendering/ArchitecturePhase1.md)
- [Rendering architecture phase 2](Rendering/ArchitecturePhase2.md)
- [Rendering milestones](Rendering/Milestones.md)
- [Rendering phase 4](Rendering/Phase4.md)
- [Resource ownership](Rendering/ResourceOwnership.md)
- [Frame graph](Rendering/FrameGraph.md)
- [Frame graph core](Rendering/FrameGraphCore.md)
- [Frame graph transitions](Rendering/FrameGraphTransitions.md)
- [Shader graph](Rendering/ShaderGraph.md)
- [Rendering integration](Rendering/Integration.md)
- [Component render helpers](Rendering/ComponentRenderHelpers.md)
- [Rendering smoke tests](Rendering/SmokeTest.md)

### Integration

- [Integration overview](Integration/Overview.md)
- [Cross-computer integration](CrossComputerIntegration.md)
- [Merged-version verification](Integration/MergedVersionVerification.md)
- [Presentation lifecycle](Integration/PresentationLifecycle.md)
- [D3D12 memory](Integration/D3D12Memory.md)

### Editor, scene, runtime, and scripting

- [Editor architecture](EditorArchitecture.md)
- [Scene and physics](SceneAndPhysics.md)
- [Runtime logging](RuntimeLogging.md)
- [Scripting and packaging](ScriptingAndPackaging.md)
