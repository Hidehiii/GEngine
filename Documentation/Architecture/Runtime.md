# Runtime and window architecture

[Architecture overview](Architecture.md) · [Runtime API](../API/Runtime.md)

`Application` delegates to one `EngineRuntime`. The runtime owns the window,
layer stack, render system, configuration, and optional services. It is the
owner of application startup and shutdown, not a second public rendering API.

```text
Client CreateApplication
        |
   Application
        |
   EngineRuntime
   |     |       |
Window  Layers  RenderSystem
                   |
             Graphics / backend
```

## Startup and shutdown

`EngineRuntime` initializes logging, optionally changes the working directory,
deserializes `Config.ini`, configures the graphics runtime, creates a GLFW or
Win32 `Window`, initializes the render system, and then initializes input and
enabled services. `RuntimeSpecification` controls Mono scripting, PhysX,
OpenSSL, and ImGui; the ImGui layer is pushed as an overlay when enabled.
`Graphics_API` from configuration selects the backend. The window/surface
adapter owns the native window and feeds events/input into the portable
`Window` interface.

On shutdown, the runtime saves configuration, shuts down enabled scripting and
PhysX, waits for GPU work, clears layers (including ImGui), shuts down the
render system and shared graphics caches, then destroys the window/context.
Layer-owned GPU resources must be destroyed before the graphics context. See
[rendering architecture](Rendering.md) for submission and resource retirement.

## Frame and event flow

`EngineRuntime::RunFrame` updates `Time`, invokes layer `OnUpdate`, delegates
rendering to `RenderSystem`, calls `OnLateUpdate` and `OnEndFrame`, pumps window
events, and advances the graphics frame. Rendering itself acquires a swap-chain
image, records layer offscreen/UI/presentation work, and submits it; the detailed
sequence lives in [Rendering.md](Rendering.md).

Window close requests loop exit. A zero-size resize marks the runtime
minimized and skips layer/render work until a nonzero resize; other events go
through `EventDispatcher` and then through layers in reverse order until
handled. Window adapters update `Input`'s per-frame query state. Scene
lifecycle is not automatically invoked by `EngineRuntime`: an application
layer or editor drives its scenes separately.

`LayerStack` owns raw layer pointers passed to `PushLayer`/`PushOverlay` and
deletes them on clear. `Core` supplies config, logging, input, time, UUID,
CPU-buffer, AABB, and thread helpers; `Events` provides typed application,
keyboard, and mouse events. These facilities do not own the graphics device.
