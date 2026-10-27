# Runtime and window architecture

Packaged startup now has a pre-Application phase. EntryPoint calls the portable
ResourcePack interface, implemented under Platform/Windows: resolve EXE, require
its same-stem `.gpak` beside the executable or in the categorized sibling
`../Data/` directory, validate metadata/paths and per-file SHA-256,
stream into a unique writable temp session, then set cwd before configuration,
logging, shader/asset reads and Application construction. Failure never proceeds
with source-directory fallback. Development binaries without a required pack
retain their existing startup behavior. Categorized packaged games run their real
EXE from `Runtime/`, so native DLL loading remains adjacent to that executable;
the user-visible root EXE is a launcher.

The published archive is immutable. Runtime config rewrites, logs/caches and
relative writes stay in the session; sessions are retained and no cleanup thread
or new GPU ownership is introduced. This initial archive adapter extracts files
for existing readers; it is not direct archive streaming, compression, encryption
or a resource identity database. See the Runtime API for format/bounds and the
game export guide for native-only build and publication limitations.

UI dependency upgrade: Application publishes ownership of EngineRuntime before
calling its idempotent InitializeImGui step. Backend attach can then safely use
Application window/config getters. Runtime construction alone does not attach UI.

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
`ScriptRuntimeRoot` and `ScriptCoreAssembly` are resolved against
`WorkingDirectory` before the runtime changes the process working directory,
then passed to `ScriptEngine` as absolute paths.
`Graphics_API` from configuration selects the backend. The window/surface
adapter owns the native window and feeds events/input into the portable
`Window` interface.

On shutdown, the runtime saves configuration, shuts down enabled scripting and
PhysX, waits for GPU work, clears layers (including ImGui), shuts down the
render system and shared graphics caches, destroys the window/context, and
flushes and closes its loggers.
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

## Runtime logging

`GE_ENABLE_LOCAL_LOG_SAVE` defaults to 1 in the public log header. When the
engine is compiled with 0, `Log::InitFileLogging` skips directory creation and
the rotating file sink. Console logging and level filtering remain active;
shutdown still flushes both loggers. The value must be set for the engine
compilation, not only for a separately compiled client application.
`Tests/LocalLogSave.cpp` directly compiles the log implementation under each
switch value to check this ownership boundary.
Debug and Release focused runs passed for both values; the full example was
rebuilt and run only with the default enabled value.
The user subsequently reported completing the macro-off follow-up test; its
per-configuration output was not captured by the agent. This changes the
verification status, not the runtime architecture.

`Log::Init` creates stdout-color spdlog loggers named `GEngine` and `APP`
before the optional working-directory change, so early diagnostics reach
stdout. `EngineRuntime` then calls `Log::InitFileLogging`: both loggers share
one synchronous, thread-safe rotating sink. The default destination is
`Logs/GEngine.log` relative to the resolved working directory, with 5 MiB per
file and three rotated files. File records contain date/time, level, logger
name, and message. Info and higher levels flush promptly; normal runtime
shutdown flushes both loggers after graphics and window teardown. If opening
the file fails, the sink reports a warning and stdout remains available.
The example's optional `GENGINE_EXAMPLE_LOG_STRESS=1` workload writes from two
CPU workers at frame 30, allowing rotation/concurrency checks without normal
per-frame spam. Its verification status is tracked in
[RuntimeLogging.md](../../TODO/RuntimeLogging.md). An optional live
in-application viewer is a plan-only follow-up in that TODO.
Debug D3D12 confirmed the file was readable before process exit, concurrent
writes, rotation, and console fallback. Release D3D12, OpenGL, and Vulkan
completed 120-frame noninteractive runs with runtime logs and GPU readback;
visible presentation and long-run memory stability remain untested.
