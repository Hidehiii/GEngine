# Engine architecture

This is the architecture index for the current first-party engine source tree.
Each function has a focused page:

- [Runtime and window](Runtime.md): startup, shutdown, layers, time, input,
  events, and surface adapters.
- [Scene and services](SceneAndServices.md): objects/components, 2D and 3D
  physics, scripting, assets, tools, ImGui, math, utilities, and diagnostics.
- [Rendering and backends](Rendering.md): frame graph, device/queues,
  presentation, OpenGL/Vulkan/D3D12 boundaries, and GPU lifetime.
- [Build system](BuildSystem.md): project generation and build output boundaries.
- [Qt editor architecture](QtEditor.md): Qt Host, diagnostic IPC and E0.2 engine
  preview/logs; planned scene ownership, interactive viewports and migration criteria.
- [Legacy editor removal](Editor.md): removal scope and links to the Qt replacement.

Caller-facing entry points are in the [API index](../API/README.md). The
[Standard workflow](../Workflow/StandardWorkflow.md) requires architecture and
API documents to stay synchronized with code and plans. These pages describe
implemented behavior; every TODO or planned task is recorded in `TODO/`, and
these pages may link to but must not own that plan. The editor proposal is
explicitly a design document; its implementation work is owned by
[editor tracking](../../TODO/EditorArchitecture.md).

## Source layout

| Location | Responsibility |
| --- | --- |
| `GEngine/include/GEngine` | Portable public headers and runtime interfaces. |
| `GEngine/src/GEngine` | First-party implementation and backend-neutral orchestration. |
| `GEngine/include/Platform`, `GEngine/src/Platform` | OpenGL, Vulkan, and D3D12 implementations. |
| `GEngine/include/Surface`, `GEngine/src/Surface` | GLFW and Win32 window/input implementations. |
| `Example` | Focused runnable consumers of engine APIs. |
| `GEngine-Editor` | Separate editor application built on the engine. |
| `GEngine/GEngine-ThirdParty` | Pinned aggregate third-party dependency submodule. |

## Subsystem directory

Rows correspond to matching first-party directories under
`GEngine/include/GEngine` and `GEngine/src/GEngine`. A public header is an
interface, not proof that all declared operations are complete.

| Subsystem | Architectural home | API home |
| --- | --- | --- |
| Root `Application`, `Layer`, `LayerStack`, `Window`; `Runtime` | [Runtime](Runtime.md) | [Runtime API](../API/Runtime.md) |
| `Core`, `Events` | [Runtime](Runtime.md) | [Runtime API](../API/Runtime.md) |
| `Graphics`, `Renderer`, `Compute` | [Rendering](Rendering.md) | [Graphics API](../API/GraphicsAPI.md), [Render graph](../API/RenderGraph.md) |
| `Object`, `Components`, `Physics`, `Scripting` | [Scene and services](SceneAndServices.md) | [Scene and physics API](../API/SceneAndPhysics.md), [Assets and services API](../API/AssetsAndServices.md) |
| `Asset`, `Tools`, `Math` | [Scene and services](SceneAndServices.md) | [Assets and services API](../API/AssetsAndServices.md) |
| `ImGui`, `Debug`, `Utils` | [Scene and services](SceneAndServices.md) | [Assets and services API](../API/AssetsAndServices.md) |
| Backend `Platform` and `Surface` | [Rendering](Rendering.md), [Runtime](Runtime.md) | Portable `Graphics` and `Window`; native headers are backend-facing. |

`GEngine/include/GEngine.h` aggregates common application headers but not
every public facility. Include specific headers for `RenderGraph`,
`EngineRuntime`, `ScriptEngine`, or tools as needed.
