# Runtime, core, window, and event API

`Application` initializes optional ImGui after assigning its runtime. Direct
`EngineRuntime` consumers must call `InitializeImGui()` after publishing any
Application facade needed by UI backends; repeated calls are harmless. UI backends
currently require that facade. Non-UI construction behavior is unchanged.

[API index](README.md) · [Runtime architecture](../Architecture/Runtime.md)

## Application and layers

Implement `CreateApplication(ApplicationCommandLineArgs)` in the client and
construct `Application` with `ApplicationSpecification`. Its fields include
name, window size, working directory, command-line arguments, script runtime
root, script core assembly, and switches for scripting, physics, OpenSSL, and
ImGui. Relative script paths resolve against `WorkingDirectory` before the
runtime changes the process working directory. `PushLayer` and `PushOverlay` transfer
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
graphics viewport and notifies the presenter. Vulkan checks extent before
acquire; OpenGL updates its presentation viewport; D3D12 now rebuilds its
swap-chain buffers and targets on nonzero size changes. The user's manual
D3D12 resize check passed, while detailed debug/memory evidence remains open
in the [rendering TODO](../../TODO/RenderingArchitecture.md).

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

`Log::Init()` initializes stdout-color `GEngine` and `APP` loggers; the
`GE_CORE_*` and `GE_*` macros write at their named levels without signature
changes. `EngineRuntime` calls `Log::InitFileLogging()` after resolving
`WorkingDirectory` and `Log::Shutdown()` after window teardown. Both loggers
then also write to `Logs/GEngine.log` relative to that directory. The file
records date/time, level, logger name, and message. It rotates at 5 MiB per
file, retaining three rotated files in addition to the active file. Info and
higher levels flush promptly; shutdown flushes and closes the loggers. Early
startup output remains stdout-only. An unusable file path produces a warning
on stdout and does not stop the runtime. There is no in-application viewer.

`GE_ENABLE_LOCAL_LOG_SAVE` defaults to 1 in `<GEngine/Core/Log.h>`. Define
`GE_ENABLE_LOCAL_LOG_SAVE=0` when **building GEngine** to disable local file
saving without disabling console records or `GENGINE_LOG_LEVEL`. Defining it
only while compiling an application against an already-built GEngine binary
does not change that binary's file sink. With 0, the file path/rotation
environment overrides are ignored and no `Logs` directory is created by the
logger.
The focused `Tests/LocalLogSave.cpp` check can confirm both build-switch
branches without launching a graphics backend.
Debug and Release checks passed for both macro values; the full D3D12 example
was rebuilt and run with the default enabled value. A full backend build with
the disabled value was not performed by the agent. The user later reported the
follow-up test done; no per-configuration output was supplied. This is a
verification-status update, not an API change.
There is no in-application viewer. The optional follow-up to add one is
tracked in [RuntimeLogging.md](../../TODO/RuntimeLogging.md), not exposed as
an implemented API.

Optional process environment overrides: `GENGINE_LOG_FILE` sets the path
(relative paths use the resolved working directory); `GENGINE_LOG_LEVEL` accepts
`trace`, `debug`, `info`, `warn`, `error`, `critical`, or `off` (default `trace`);
`GENGINE_LOG_MAX_BYTES` accepts 1024 through 1073741824 (default 5242880);
`GENGINE_LOG_MAX_FILES` accepts 1 through 10 (default 3). Invalid level or
numeric values warn and use their defaults. See the
[logging TODO](../../TODO/RuntimeLogging.md) for verification status.
`FrameGraphTriangle` also accepts `GENGINE_EXAMPLE_LOG_STRESS=1` as an opt-in
example-only check: at frame 30, two workers write core messages, surrounded
by application messages. This is not a general runtime option.
Debug D3D12 testing confirmed the log file while running, rotation, and
stdout-only fallback for an unusable path. Release D3D12, OpenGL, and Vulkan
120-frame runs completed with runtime logs and GPU readback. These
noninteractive checks do not verify visible presentation.
