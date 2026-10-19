# GEngine Documentation

This directory documents the engine as it exists in the current source tree. It is intended for application and engine contributors; public headers in `GEngine/include` remain the source of truth for API signatures.

## Contents

- [Engineering workflow](../AGENTS.md): repository-wide implementation, GPU lifetime, verification, example, documentation, and Git standards for coding agents.
- [Standard workflow](Workflow/StandardWorkflow.md): required architecture and API documentation updates for every code or plan change.
- [Cross-computer workflow](Workflow/CrossComputerWorkflow.md): preserving work, integrating branches and validating the merged renderer.
- [Focused regression checks](../Tests/README.md): no-PCH public logging-header compilation and formatting checks.
- [Architecture index](Architecture/Architecture.md): function-focused runtime, scene/services, and rendering/backend architecture.
- [API index](API/README.md): function-focused public entry points across runtime, scene, physics, graphics, compute, assets, tools, and diagnostics.
- [Features and samples](Guides/Features.md): implemented subsystems, backend status, configuration, and example projects.
- [Build and run](Guides/BuildAndRun.md): prerequisites, solution generation, configuration selection, and common build failures.
- [Shader authoring](API/ShaderAuthoring.md): `.shader` pass syntax, HLSL stages, reflection and vertex input rules.
- [Render graph](API/RenderGraph.md): declaring pass dependencies and resource states.
- [D3D12 diagnostics](Diagnostics/D3D12Diagnostics.md): validation, RenderDoc workflow and common failure patterns.

## Getting started

1. Generate a Visual Studio solution with `BuildScripts/GenerateProjectsPremake.bat` (VS2026, VS2022, VS2019) or `BuildScripts/GenerateProjectsCMake.bat` (VS2022, VS2019). See [Build and run](Guides/BuildAndRun.md) for usage and parity limits.
2. Build one configuration at a time. Do not start a second build for the same configuration and output directory: MSVC's shared precompiled-header file cannot be written concurrently.
3. Build and run `Example/Triangle` to validate the basic graphics path.

The Triangle sample uses the rendering path in the [graphics and compute API guide](API/GraphicsAPI.md) and is the smallest D3D12 rendering reference in the repository.
