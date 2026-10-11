# Runtime, core, window, and event API

[API index](README.md) · [Runtime architecture](../Architecture/Runtime.md)

## Application and layers

Implement `CreateApplication(ApplicationCommandLineArgs)` in the client and
construct `Application` with `ApplicationSpecification`. Its fields include
name, window size, working directory, command-line arguments, and switches for
scripting, physics, OpenSSL, and ImGui. `PushLayer` and `PushOverlay` transfer
layer lifetime to the runtime's `LayerStack`; `Close()` requests loop exit.
Layer hooks include `OnAttach`, `OnUpdate`, `OnRender`, `OnImGuiRender`,
`OnPresent`, `OnLateUpdate`, `OnEndFrame`, and `OnEvent`. `OnPresent` draws to
the swap chain; `OnRender` precedes it for offscreen work.

`<GEngine.h>` includes common application headers. For direct runtime access,
include `<GEngine/Runtime/EngineRuntime.h>`; normal applications use
`Application` rather than owning an `EngineRuntime` separately.

## Configuration, window, input, and events

`Config.ini` is loaded through `Serializer` at startup and saved on
shutdown. `Config` exposes graphics API, frame count, VSync,
command-buffer count, shader cache directory, input timing thresholds, and
window-manager selection. `Window` abstracts the current native window;
`WindowManagerAPI` chooses GLFW or Win32.

Use `EventDispatcher` for typed window, key, and mouse events. For frame
queries, `Input::GetKeyDown/GetKeyPressed/GetKeyUp` and the mouse equivalents
read state updated by the window adapter. `Time::GetDeltaTime`,
`GetFixedTime`, and `GetRunTime` read runtime-maintained clocks. The
[configuration guide](../Guides/Features.md) lists important keys, and
[InputSystem](../../Example/InputSystem) is an example project.

`Application::OnEvent(Event&)` forwards a `WindowResizeEvent` to the runtime:
zero width or height pauses rendering, while a nonzero size updates the
graphics viewport. This public event path is not a guarantee that every
presenter has rebuilt its swap chain. Vulkan checks extent before acquire;
OpenGL updates its presentation viewport; D3D12 currently lacks an explicit
swap-chain resize path. See the
[rendering architecture](../Architecture/Rendering.md) and its open TODO before
relying on interactive D3D12 resize.

For a noninteractive example run, `GENGINE_EXAMPLE_SECONDS` limits execution
by elapsed time and `GENGINE_EXAMPLE_FRAME_LIMIT` limits rendered frames;
positive limits request normal `Application::Close()`. Launch
`FrameGraphTriangle` with `Example/FrameGraphTriangle` as its working directory
so `Config.ini` and shader paths resolve. Its frame-119 compute and texture
array readbacks check GPU data, not visible presentation, resize, or
minimize/restore. `GENGINE_EXAMPLE_NO_REPLACEMENT=1` disables periodic example
resource replacement for a memory comparison. These are example diagnostics,
not general `Application` options.

## Core value and utility types

`Ref<T>` and `Scope<T>` alias `shared_ptr` and `unique_ptr`. `UUID`
provides identity values; `Buffer` stores CPU bytes with explicit
`Allocate`/`Release` and `Buffer::Copy` for owned copies. `AABB` provides
geometry bounds, `Log` supplies core/client logging, and `Timer` helps with
timing. `Thread` and `TripleAtomicBuffer` are lower-level CPU concurrency
facilities, not the GPU queue interface. Public declarations are under
`<GEngine/Core/...>` and `<GEngine/Events/...>`.
