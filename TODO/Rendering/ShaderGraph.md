# Shader graph

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
