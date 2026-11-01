# Shader graph

## Runtime compiler recovery (2026-11-01)

P1 pipeline selection's required HLSL-failure repair and acceptance are owned by
[RenderPipeline P1](RenderPipeline.md). Failed DXC stages now throw before
reflection/cache/backend creation; SPIR-V failure cannot be overwritten by DXIL
success, and operation result/blob references are scoped. This changes runtime
error recovery, not shader graph authoring. Verification is recorded in P1.

- [x] Harden the remaining legacy shader-container parser separately. Concrete
  source paths still use assertions for malformed Cull/DepthTest/DepthWrite/
  pragma commands and unknown blend factors; missing structural delimiters are
  not comprehensively validated. Acceptance: invalid containers/directives fail
  with diagnostics in Debug/Release without assertion termination, invalid
  indexing or cache publication; valid existing shaders retain behavior on all
  backends, and pipeline selection preserves its old instance on these failures.
  This is a registered follow-up, not additional implementation authorized by P1.
  Rewrite authorized by the user (2026-11-01). Bounded scope: a CPU-only,
  backend-neutral container parser with token/line diagnostics, validated render
  state and stage declarations, comment/string-aware balanced Program extraction,
  complete-input validation and transactional parse results. Shader loading must
  finish parsing before cache access/publication or backend compilation. Preserve
  existing Shader/Pass/Program syntax, defaults, aliases, source bytes and valid
  assets; accept documented DepthWrite On/Off correctly. Reject malformed and
  duplicate directives/stages instead of assertions or silent defaults. Add CPU
  Debug/Release regressions and corpus checks, plus a real example exercising
  malformed-candidate rollback and unchanged valid output. Update API/architecture,
  authoring and example docs. Current host's D3D12 build exclusion stays active;
  attempt normal engine/example builds and report that blocker, while running
  parser-only checks independently without native runtime dependencies. Do not
  expand to visual shader graphs, HLSL compilation or cache-format redesign.
  Implementation (2026-11-01): new ShaderContainer.h/.cpp scanner/parser returns
  complete owned snapshots; Shader::Preprocess constructs temporary state/stage/
  mesh/source vectors and commits with swaps. Disk-cache access/folder creation
  follows successful parsing; existing in-memory path/name reuse is unchanged.
  Source fingerprint now includes parser revision 2 to invalidate old compiled
  extraction results. Unknown state conversion helpers throw; DepthWrite On/
  True/1 enables writes and Off/False/0 disables them. Full-input structure,
  duplicates, entry identifiers, stage combinations and Program boundaries are
  checked with source/line/column errors. No native backend code changed.
  CPU verification: VS2022 standalone production-parser Debug/Release builds
  and semantic tests pass, including exact Program bytes/final character,
  defaults/aliases, CRLF/BOM, comment/string/escaped quote/character handling,
  multiple passes and 2160 malformed/truncated/deterministic generated inputs.
  Both corpus runs accept 17 current-syntax payloads (including the deliberately
  HLSL-invalid but structurally valid compiler fixture), reject 28 old #Name GLSL
  containers/mislabeled fragments and reject both new malformed fixtures. Legacy
  formats were already incompatible with Shader/Pass/Program; no compatibility
  success is claimed for them. Manifest names every source/result. An initial
  corpus treating those old formats as valid failed and was corrected to explicit
  rejection coverage rather than silently skipping them.
  Logs: build/shader-container/{configure,build-Debug,build-Release}.log and
  corpus-{Debug,Release}/{manifest.json,corpus.log}. Standalone headers export
  Log accessors, so the CPU harness provides their storage; the first link lacked
  those globals and failed, then both builds passed. Existing Core/Log.h C4251
  export warnings remain visible. No logger/device/Application is initialized.
  Shader.cpp, ShaderContainer.cpp and PipelineAssetChecks.cpp also pass MSVC /Zs
  syntax checks in Debug/Release using generated engine include/define settings
  and isolated VS environments (syntax-*.log); no engine/native executable was
  produced by those checks. The initial ad-hoc syntax invocation lacked /MDd/
  NDEBUG settings required by PhysX, and was corrected before successful runs.
  Example implementation: --parser extends P1 with invalid DepthWrite and missing
  root-brace candidates, each retried twice, asserting original pipeline/settings
  retention and no fixture cache folders; expected rejection count is 22 plus
  existing valid pixel/switch/resize/discard/shutdown checks. Original modes keep
  their old counts. Runner syntax and both fixture payloads pass CPU checks.
  Normal engine/RenderPipeline Debug and Release build attempts configure but
  fail on the existing excluded system SDKLayers 10.0.26100.4768, as required.
  Logs: build/pipeline-lifecycle/build-RenderPipeline-{Debug,Release}.log.
  No current OpenGL, Vulkan shared/dedicated or D3D12 example runtime/readback/
  rollback/cache-publication result is claimed. Keep this item open for those
  end-to-end Debug/Release checks after the host runtime is changed. No disabling
  guard, host update, backend selection change, commit or push performed.
  Final handoff checks: repeated Debug/Release semantic/corpus runs pass after
  the escaped-string and exact source-location checks. Both new fixture payloads
  are included even before staging. Premake VS2022 regeneration succeeds and
  generated engine projects include ShaderContainer.h/.cpp; CMake discovery also
  includes the new source. Python runner syntax, 43 relative documentation links
  and git diff --check pass. Source/integration diff reviewed; unstaged review
  refresh queued. Existing tmp/ remains the only unrelated content.
  Later host setup (2026-11-01): Graphics Tools is Installed with system
  SDKLayers 10.0.26100.1 and the unchanged build guard passes. The historical
  build blocker above is cleared; engine/example rebuild and backend runtime
  acceptance remain pending. See ../Integration/D3D12Memory.md for setup evidence.
  Final integration verification after reinstall: engine, FrameGraphTriangle
  and RenderPipeline build in Debug/Release. Each configuration passes --parser
  separately on OpenGL, Vulkan shared, Vulkan dedicated and D3D12: 4 malformed
  rollback retries, 22 total rejections, no malformed fixture cache, 7 valid
  switches, 9 numeric probes and normal exit. Logs are under
  build/graphics-tools-install/verification/parser-{Debug,Release}/. Bounded
  parser rewrite acceptance is now complete. The independent D3D12 validation
  handle leak still reproduces and remains owned by Integration/D3D12Memory.md.

## Planned: editor-facing shader graph (2026-10-27)

- [ ] Design and implement a visual shader graph authoring workflow. Scope:
      define the graph, node, pin, and connection data model; persistence and
      versioning; validation and compilation into the existing `.gasset`
      shader envelope (whose payload uses the `.shader` pass/HLSL format);
      editor ownership and integration with the Qt Host;
      and backend-neutral support for OpenGL, Vulkan, and D3D12 shader stages.
      Do not conflate this with the runtime render graph, which declares GPU
      passes and resource dependencies rather than material shader logic.
      Initial scope should choose a bounded milestone (for example, a static
      material graph with inputs, constants, basic arithmetic/swizzle nodes,
      and one fragment output) before general control flow, subgraphs, or
      dynamic node registration.
      Acceptance: a focused example authored as a graph produces the same
      generated `.gasset` shader semantics as a hand-written reference; Debug/Release
      builds pass with generated shaders on each available backend; invalid
      connections, cycles, missing pins, unsupported stage combinations, and
      schema-version failures are reported without launching compilation;
      graph save/load preserves topology, metadata, and validation state; the
      Qt editor can create/edit/save/reload a graph; source integration,
      generated-code inspection, and asset-cache behavior are documented; and
      architecture/API pages plus the example README are synchronized. GPU
      visual verification must be recorded separately from compilation checks.
      Remaining: no implementation is authorized by this TODO entry alone.
      2026-10-29 design direction (TODO registration only): the graph source
      should use a versioned editor-owned format such as `.shadergraph`, while
      deterministic code generation targets the existing `.gasset` envelope
      with `type: shader` and a `.shader` authoring payload, reusing the
      reflection/cache/backend pipeline. The runtime should not parse editor
      graph data, node layouts, or UI metadata. Keep hand-authored `.shader`
      files as an advanced compatibility input; generated `.gasset` files must
      be inspectable but not treated as graph round-trip sources. Graph cache
      identity must include the graph schema, graph content, code-generator
      version, and active backend target. Backend-unsupported nodes or stage
      combinations must fail explicitly rather than silently degrade.
