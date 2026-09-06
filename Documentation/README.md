# GEngine Documentation

This directory documents the engine as it exists in the current source tree. It is intended for application and engine contributors; public headers in `GEngine/include` remain the source of truth for API signatures.

## Contents

- [Architecture](Architecture.md): runtime ownership, frame flow, queues, shutdown ordering, renderer, and backend boundaries.
- [Graphics API](GraphicsAPI.md): creating shaders, materials, buffers, pipelines, and a presentable draw.
- [Features and samples](Features.md): implemented subsystems, backend status, configuration, and example projects.
- [Build and run](BuildAndRun.md): prerequisites, solution generation, configuration selection, and common build failures.
- [Shader authoring](ShaderAuthoring.md): `.shader` pass syntax, HLSL stages, reflection and vertex input rules.
- [Render graph](RenderGraph.md): declaring pass dependencies and resource states.
- [D3D12 diagnostics](D3D12Diagnostics.md): validation, RenderDoc workflow and common failure patterns.

## Getting started

1. Generate the Visual Studio solution with `Scripts/GenerateProject.bat`.
2. Build one configuration at a time. Do not start a second build for the same configuration and output directory: MSVC's shared precompiled-header file cannot be written concurrently.
3. Build and run `Example/Triangle` to validate the basic graphics path.

The Triangle sample uses the API in [Graphics API](GraphicsAPI.md) and is the smallest D3D12 rendering reference in the repository.
