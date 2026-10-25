# Engine API index

These function-focused guides cover the first-party public engine surface.
The matching [architecture index](../Architecture/Architecture.md) explains
ownership and implementation boundaries. Public headers under
`GEngine/include/GEngine` remain the source of truth for signatures; an
exposed declaration does not necessarily mean a complete runtime path.

| Function | Guide |
| --- | --- |
| Application/runtime, config, window, input, events, core values | [Runtime API](Runtime.md) |
| Scene, game objects, components, 2D/3D physics | [Scene and physics API](SceneAndPhysics.md) |
| Math, mesh/assets, serialization, tools, ImGui, scripting, utilities, diagnostics | [Assets and services API](AssetsAndServices.md) |
| Graphics device, resources, queues, pipelines, compute, presentation | [Graphics and compute API](GraphicsAPI.md) |
| GPU pass/resource dependency declarations | [Render graph API](RenderGraph.md) |
| Shader source container and authoring | [Shader authoring](ShaderAuthoring.md) |
| Contributor-facing project generation | [Build entry points](BuildSystem.md) |
| Planned C# runtime paths, resource deployment and archive boundaries | [Managed scripting contracts](ManagedScripting.md) |
| Qt Host diagnostic/engine preview IPC and planned scene contracts | [Qt editor contracts](QtEditor.md) |
| Legacy editor removal and replacement | [Removed editor interfaces](Editor.md) |

`<GEngine.h>` aggregates common application, scene, math, graphics, and
selected tool headers; include a specific public header for `RenderGraph`,
`EngineRuntime`, `ScriptEngine`, or other facilities not in the umbrella.
`Ref<T>` and `Scope<T>` alias `shared_ptr` and `unique_ptr`.

Backend `Platform` and window `Surface` headers are implementation-facing.
Use the portable `Graphics`/`RenderDevice` and `Window` interfaces in
applications. Plans and limitations must be labeled separately from verified
behavior. Every TODO or planned task is recorded in `TODO/`; a guide may link
to the owning entry but must not be its sole record, following the
[Standard workflow](../Workflow/StandardWorkflow.md).
