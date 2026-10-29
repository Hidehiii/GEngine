# GEngine Documentation

This document is the entry point for project documentation. It links every
current Documentation page and the repository's project-level guides. Public
headers in `GEngine/include` remain the source of truth for API signatures.

## Project documentation

### Workflow and engineering

- [Engineering workflow](../AGENTS.md): repository-wide implementation, GPU lifetime, verification, example, documentation, and Git standards for coding agents.
- [Standard workflow](Workflow/StandardWorkflow.md): required architecture and API documentation updates, with every TODO or planned task recorded in `TODO/`.
- [Cross-computer workflow](Workflow/CrossComputerWorkflow.md): preserving work, integrating branches and validating the merged renderer.
- [TODO workflow index](../TODO/Workflow.md): current work-item topics, including build, rendering, editor, integration, scripting, and runtime tracking.
- [Focused regression checks](../Tests/README.md): no-PCH public logging-header compilation and formatting checks.

### Architecture

- [Architecture index](Architecture/Architecture.md): subsystem map and source layout.
- [Runtime and window](Architecture/Runtime.md): startup, shutdown, layers, events, time, and surfaces.
- [Scene and services](Architecture/SceneAndServices.md): objects, components, physics, scripting, assets, tools, and utilities.
- [Rendering and backends](Architecture/Rendering.md): rendering architecture entry point.
- [Build system](Architecture/BuildSystem.md): project generation, native workflow tool, and output boundaries.
- [Managed scripting](Architecture/ManagedScripting.md): Mono runtime, deployment, and packaging architecture.
- [Qt editor](Architecture/QtEditor.md): Qt Host, EngineWorker, documents, projects, and preview architecture.
- [Legacy editor removal](Architecture/Editor.md): removed editor scope and replacement.

#### Rendering details

- [Backends](Architecture/Rendering/Backends.md)
- [Extensions](Architecture/Rendering/Extensions.md)
- [Framework](Architecture/Rendering/FrameWork.md)
- [GPU lifetime](Architecture/Rendering/Lifetime.md)
- [Queues](Architecture/Rendering/Queues.md)

### API

- [API index](API/README.md): public entry-point map.
- [Runtime API](API/Runtime.md)
- [Scene and physics API](API/SceneAndPhysics.md)
- [Assets and services API](API/AssetsAndServices.md): GAsset envelope, mesh/assets, serialization, tools, ImGui, scripting, utilities, diagnostics.
- [Graphics and compute API](API/GraphicsAPI.md)
- [Render graph API](API/RenderGraph.md)
- [Shader authoring](API/ShaderAuthoring.md)
- [Build entry points](API/BuildSystem.md)
- [Managed scripting contracts](API/ManagedScripting.md)
- [Qt editor contracts](API/QtEditor.md)
- [Removed editor interfaces](API/Editor.md)

#### Graphics details

- [Application](API/Graphics/Application.md)
- [Application draw](API/Graphics/ApplicationDraw.md)
- [Commands](API/Graphics/Commands.md)
- [Device](API/Graphics/Device.md)
- [Diagnostics](API/Graphics/Diagnostics.md)
- [Resources](API/Graphics/Resources.md)
- [Shader assets](API/Graphics/ShaderAsset.md)

#### Render graph details

- [Basic pattern](API/RenderGraph/BasicPattern.md)
- [Execution](API/RenderGraph/Execution.md)
- [Presentation](API/RenderGraph/Presentation.md)
- [Resources](API/RenderGraph/Resources.md)
- [Transient resources](API/RenderGraph/TransientResources.md)

### Guides and diagnostics

- [Features and samples](Guides/Features.md): implemented subsystems, backend status, configuration, and example projects.
- [Build and run](Guides/BuildAndRun.md): prerequisites, fresh-environment quick start, CMake/Premake generation, and common failures.
- [Game export](Guides/GameExport.md): project packaging, `.gpak`, publication, and native runtime.
- [D3D12 diagnostics](Diagnostics/D3D12Diagnostics.md): validation, RenderDoc workflow and common failure patterns.

### Project references

- [GEngine editor](../GEngine-Editor/README.md)
- [FrameGraphTriangle example](../Example/FrameGraphTriangle/README.md)
- [Scene render helpers example](../Example/SceneRenderHelpers/README.md)
- [Deferred render example introduction](../Example/DeferredRender/intro.md)

## Getting started

1. Generate a Visual Studio solution with `BuildScripts/GenerateProjectsPremake.bat` (VS2026, VS2022, VS2019) or `BuildScripts/GenerateProjectsCMake.bat` (VS2022, VS2019). See [Build and run](Guides/BuildAndRun.md) for usage and parity limits.
2. Build one configuration at a time. Do not start a second build for the same configuration and output directory: MSVC's shared precompiled-header file cannot be written concurrently.
3. Build and run `Example/Triangle` to validate the basic graphics path.

The Triangle sample uses the rendering path in the [graphics and compute API guide](API/GraphicsAPI.md) and is the smallest D3D12 rendering reference in the repository.
