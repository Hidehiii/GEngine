# FrameGraphTriangle

The included `FrameGraphTriangle.gproject` can be opened in the Qt editor through
File > Open project. Project > Package project asks for output directory,
executable name and optional logo image,
then automatically compiles the project and engine and publishes a root
FrameGraphTriangle.exe with Runtime/, Data/, and Metadata/ folders. The descriptor has
only schemaVersion/name. No existing executable or packaging fields are needed.
Run the generated root EXE directly; it mounts the validated archive and reads
assets on demand. Config/Config.ini and Cache/Shaders/<API>/ live directly under
the game root, along with Logs/ and UserData/. Configuration is not in .gpak.
The output has no loose Assets directory or script launcher.

For current automatic packaging, see the [export guide](../../Documentation/Guides/GameExport.md).
The former directory-deployment smoke scripts were removed. The native
`.gproject` pipeline now compiles the project, creates the `.gpak`, publishes
the categorized output, and validates repeated builds through the editor's
`--packaging-self-test`.

For the custom-name/logo regression, run the built Qt Host with
`--packaging-self-test` (or CTest `EditorPackaging`). It builds `TriangleGame.exe`,
checks the embedded blue/white test logo pixels and matching runtime/pack/manifest,
then launches the root entry from an unrelated working directory for 120 frames.
Expected: normal exit and all three frame-119 readbacks. It also checks zero-copy
repeat, failure preservation, cancel/retry, renaming to `Renamed Game`, clearing
the logo and reopening preferences. Missing icons, old filenames, changed project
metadata or nonzero copies on an unchanged repeat fail the check. This packaging
test does not exercise manual resize; rendering resize/replacement/shutdown checks
remain separate below.

Set `GENGINE_EXAMPLE_RESOURCE_CHECK=1` with `GENGINE_EXAMPLE_FRAME_LIMIT=120` to
check owned background reads, 2x2 PPM decoding/upload, FBX memory-stream import and
scene YAML deserialization. ResourceProbe.fbx is copied from the existing
GpuDriven cube fixture. The compute shader also reads its HLSL include through
the resource interface. Expected: the usual triangle and color changes, all three
GPU readbacks, resource-check messages and normal shutdown; a packaged run must
have no Assets directory even after shader compilation. Cache/Shaders/<API>/ and Logs
remain writable in the game root. This check uses the same logical paths in
development mode. It does not validate manual resize or full material restoration.

`python Tests/ResourcePackRuntime.py <package-root-exe> --logs <log-directory>`
temporarily selects each backend through external Config/Config.ini, runs cold and
warm launches, verifies unchanged nonempty cache files and include invalidation,
rejects corrupt/missing packs and missing external config, and restores config/pack.
It preserves source
Config.ini; run it on a test package with no concurrent users. Focused malformed
archive/concurrency regression commands are in [Tests](../../Tests/README.md).

Set `GENGINE_EXAMPLE_IMGUI=1` to enable the dependency-upgrade UI smoke window.
Expected: readable version text, a working click counter, and a window that can
be dragged outside the main viewport. Check resize and normal shutdown on each
backend; missing text, assertions, validation errors, or crashes indicate failure.
This checks fonts and backend UI submission, not editor image/gizmo behavior.

The main UI pass renders to an offscreen target; this example still presents its
triangle. A normal exit validates submission and teardown, not visible UI pixels.
Visual inspection, actual detached-window interaction, and long-run memory remain
separate manual checks. All twelve Debug/Release graph/UI backend runs exited 0
after 120 frames during the local 2026-10-22 upgrade validation.

Dependency upgrade validation: this example exercises SPIRV-Cross reflection and
the OpenGL SPIR-V-to-GLSL path. Run Debug and Release on each backend using the
bounded run instructions below. It does not validate editor docking, image
buttons, fonts, or gizmo manipulation; those require separate editor checks.

This is a GPU graph and presentation regression example. It renders a triangle
whose orange/green color is computed through public engine APIs; it does not include D3D12,
Vulkan, or OpenGL headers and it does not access native handles.

The graph records graphics -> compute -> graphics work, owns the offscreen
attachments and joins every submission before presentation. The
engine's `RenderSystem` builds the following per-frame graph around the layer
draw:

```text
LayerRender (graph: read color -> compute color -> read updated color)
    -> Present (visible triangle draw) -> ReleasePresentation
                                      Present -> RenderTarget -> Present
```

Run the project after selecting the desired `GraphicsAPI` in `Config.ini`.
The runtime keeps stdout logging and writes core/application records to
`Logs/GEngine.log` under this example's working directory. Set
`GENGINE_LOG_LEVEL=info` to suppress trace/debug records or
`GENGINE_LOG_FILE` to choose another path. File records contain timestamp,
level, logger name, and message; the default rotates at 5 MiB and retains
three old files. `GENGINE_LOG_MAX_BYTES` and `GENGINE_LOG_MAX_FILES` override
those limits. To turn off local saving at compile time, build **GEngine** with
`GE_ENABLE_LOCAL_LOG_SAVE=0`; this retains console messages and level filtering.
Setting the macro only in this example cannot change an already-built engine.
`GENGINE_EXAMPLE_LOG_STRESS=1` emits core records from two worker
threads at frame 30, bracketed by application records. With
`GENGINE_LOG_MAX_BYTES=1024`, a run of at least 31 frames exercises rotation;
inspect the active and `.1.log` files while the process runs and after normal
shutdown. This diagnostic is opt-in; ordinary frames do not generate it.
An invalid or unwritable log path should warn on stdout without stopping the
example. This check does not require Visual Studio or desktop interaction.
At frame 119 a one-time synchronous readback verifies the compute-written color
against an expected value different from the original CPU value. Success logs
`GPU graph compute readback passed after 119 frames.` A mismatch throws before
the next frame. Use a frame limit of at least 120 to include this check.
Close it after several seconds as a quick check that command buffers, presenter
fences/events, and cached render resources are released while the graphics
context is still alive.  It also exercises the record-then-submit frame path:
drawing ends command recording first, and the presenter submits it only after
swapchain synchronization is assembled. On Vulkan, this additionally verifies
that the presentation submit waits for the acquire semaphore and the off-screen
submission dependency without keeping synchronization state in a command
buffer.

For non-presentation work, submit a completed command buffer through the
portable device queue:

```cpp
auto& queue = Graphics::GetRenderDevice().GetQueue(COMMAND_BUFFER_TYPE_GRAPHICS);
queue.Submit(commandBuffer);
```

Startup also declares a graph attachment through
`RenderGraph::AttachmentSpecification`. On Vulkan it requests one explicit
subpass and executes it; on OpenGL and D3D12 it verifies that subpass
semantics are rejected through capability reporting before target allocation.
Startup logs one device-derived capability snapshot and every queried source
used to derive its feature booleans.

## Resource replacement check

The merged example replaces materials, vertex buffers and graphics pipelines
every 30 frames. It also calls `SetRenderPassOperation` on the graph-owned
offscreen framebuffer to preserve the remote Vulkan render-pass retirement
regression. This runs alongside the graphics/compute graph and frame-119
numeric readback. `GENGINE_EXAMPLE_NO_REPLACEMENT` disables all these replacements
for baseline comparisons. Merged-version results are tracked separately in
[cross-computer integration](../../TODO/CrossComputerIntegration.md).

The D3D12 presentation path now rebuilds back buffers after a nonzero window
resize; zero-size/minimized windows skip rendering until restored. The example
continues to draw the offscreen graphics/compute result and presentation
triangle, so a manual resize check should see the triangle and color/motion
resume at the new size. Check both frame-119 readbacks, D3D12 debug output,
repeated resize, minimize/restore, and normal shutdown. These interactive
checks were reported successful by the user on 2026-10-18; the agent did not
control the desktop or capture manual debug/memory measurements.
For a noninteractive D3D12 path check, set
`GENGINE_EXAMPLE_SYNTHETIC_RESIZE=1` with `GENGINE_EXAMPLE_FRAME_LIMIT=120`.
After frames 30 and 60 the example sends 960x540 then 1280x720 resize
events without resizing the native window. Expect two `D3D12 swap chain
resized` messages, both frame-119 GPU readbacks, and normal shutdown.
Unset the switch for ordinary runs; this does not cover actual OS resize or
minimize/restore.
The 2026-10-18 Debug/Release 120-frame D3D12 checks logged both recreations,
passed both GPU readbacks and exited normally. Debug logged only the existing
initial UAV-state warning. OpenGL and Vulkan short runs passed without
resize, so they provide regression evidence only.

A subsequent matched 60,000-frame D3D12 Debug diagnosis used
`GENGINE_D3D12_LIFETIME_TRACE=1` and compared the Debug Layer on/off, then
kept the layer on while setting `GENGINE_D3D12_INFO_QUEUE_LOGGER=0`. All three
runs passed both readbacks and normal shutdown. Private-byte growth persisted
without the polling logger, with sampled GPU process memory and live pipeline
count stable; see the TODO for measurements and remaining ownership work.
The Debug and Release engine/example builds passed; a 120-frame Release D3D12
smoke run also passed both readbacks and normal shutdown.

Merged-version verification (2026-09-19/20, VS2022): Debug and Release builds
passed; OpenGL, Vulkan (shared and dedicated queues), and D3D12 each completed
120 frames with successful compute readback and normal shutdown. A 65-second
Debug replacement stress run on each API showed no sustained sampled memory
growth. VS2026 Debug and Release verification completed on 2026-09-21: all four
of those 120-frame backend scenarios passed, and dedicated Vulkan selected
separate graphics, compute, and transfer families. Manual visible output,
resize, and minimize remain unverified. The same Debug and Release runs
exercised the portable attachment/subpass check; Vulkan validation layers were
unavailable on this host, so those Vulkan runs fell back to no validation layer.
The capability reporting follow-up was also verified in Debug and Release on
2026-09-21: OpenGL 4.6, Vulkan 1.4.341 with both queue-selection modes, and
D3D12 feature level 12.2 each logged their queried sources and passed the
120-frame readback/shutdown regression.

Set `GENGINE_EXAMPLE_FRAME_LIMIT` to a positive frame count to request normal
shutdown after that many rendered frames; omit it (or use zero) for interactive
testing. The run logs its completed-frame count before calling Application::Close.
Latest verification (2026-09-12): the final Debug and Release engine/example
builds passed. All three backends completed 120 frames in Release with exit code
0. With `GENGINE_EXAMPLE_SECONDS=20` and no frame limit, Debug completed 49,924
OpenGL frames, 18,780 Vulkan frames and 12,172 D3D12 frames, each through normal
application shutdown. D3D12 debug logging was enabled and joined at shutdown;
no Vulkan validation errors or D3D12 errors were logged. OpenGL logged the known
shader-recompilation performance warnings. These runs exercise startup graph
regressions and repeated replacement, but do not establish visual correctness,
interactive resize, long-run memory stability or GPU compute-chain correctness.
See `TODO/RenderingArchitecture.md` for memory samples and outstanding work.

The older verification entries below are historical; the latest runs supersede
their outstanding normal-shutdown and Release smoke checks only.

The current example additionally records a real graphics -> compute -> graphics
chain through `ExecuteGpu`. Both graphics passes use a graph-created 512x512
color/depth target and read a shared raw storage buffer. Between them a compute
shader changes the buffer's red color component. Presentation reads that result
after joining the graph. The triangle alternates orange/green rapidly and still
moves left/right during pipeline replacement. No layer-side Begin/End/Submit or
native synchronization calls are used for offscreen work.

Set `GENGINE_EXAMPLE_NO_REPLACEMENT=1` to disable the 30-frame pipeline/material/
vertex-buffer replacement while preserving the same rendering workload, for
memory-growth isolation. Unset it to restore replacement. Combined with the
seconds limit, both cases request normal shutdown.
For D3D12, `GENGINE_D3D12_LIFETIME_TRACE=1` logs total-created and currently-live
pipeline counts every 1,000 creations, plus pending submissions, deferred
releases, and info-queue counts every 10,000 tracked submissions. Compare
private bytes with
`GENGINE_D3D12_DEBUG_LAYER=1` and `0`; live object counts alone do not prove that
driver allocations are reclaimed.
For an opt-in Debug D3D12 shutdown count comparison, set both
`GENGINE_D3D12_DEBUG_LAYER=1` and `GENGINE_D3D12_LIVE_OBJECTS=1`. The engine
logs available live-device-object summary messages after this example and the
renderer release their objects; context-owned objects are still live. Compare
the same report after short and long bounded runs. This does not capture CPU
allocation stacks or establish that process private-byte growth is a leak.
In a Debug Layer-on comparison, the 120- and 60,000-frame shutdown summaries
had identical object-category counts, while the long run's private bytes rose.
The layer-off 60,000-frame and 80-second controls stayed near-flat. All runs
passed the two GPU readbacks and normal shutdown. Native CPU allocation
ownership remains unresolved.
An approved 60-second WPR heap trace later found more range-end outstanding
heap allocations with the D3D12 Debug Layer on. Identified growth paths pass
through `d3d12SDKLayers.dll` and the NVIDIA user-mode driver; one is reached
from `D3D12CommandBuffer::ResetRecording`. The 1,964.3-KiB group includes
NVIDIA driver frames; a smaller 525.4-KiB `ntdll`-only group remains
unattributed, so these traces do not establish a permanent leak. See
[D3D12 diagnostics](../../Documentation/Diagnostics/D3D12Diagnostics.md).
The D3D12 storage-buffer creation path now transitions COMMON to UAV before
the graph imports the buffer as ShaderWrite. A 120-frame Debug Layer-on run
must pass both frame-119 GPU readbacks, exit normally, and show no initial
UAV-state warning. The corrected 60-second layer-on/off checks passed, but
private-byte growth persisted only with validation on; this example does not
prove a runtime/driver leak or its fix.
In a 60,000-frame Debug Layer-on isolated run with this trace, both readbacks
and shutdown passed. Pending submissions stayed at 1-10, deferred releases
at zero, and sampled GPU memory flat while process private bytes grew; see
[D3D12 diagnostics](../../Documentation/Diagnostics/D3D12Diagnostics.md).
Set `GENGINE_VULKAN_DEDICATED_QUEUES=1` for independent-family testing. Check the
selected-family log: a shared family is a compatibility run, not proof of
cross-family synchronization. The same frame-119 readback must pass.
The array-layer pixel copy now uses `BuildTransferCopyPass`: D3D12 and Vulkan
record it on copy/transfer commands, while OpenGL executes it on the graphics
context. A graphics preparatory submission establishes copy states before
dedicated transfer work. On every backend, expect the same layer-0 changed
pixel, untouched layer-1 pixel, and normal shutdown; on D3D12/Vulkan also
check debug/validation output for queue and layout errors. Vulkan resources
remain concurrently shared by default across distinct queue families.
To exercise the new opt-in Vulkan image ownership path, also set
`GENGINE_VULKAN_EXCLUSIVE_TRANSFER_IMAGES=1` with dedicated queues. The graph
should release/acquire layer 0 and the source image around the transfer copy,
then reacquire them on graphics before the readback. Expect both frame-119
readbacks, normal shutdown, and no ownership/layout validation messages.
The 2026-10-19 Debug and Release 120-frame dedicated-family runs passed both
readbacks and exited normally with no captured validation issues; a final
Debug 10,000-frame opt-in run did the same. Do not treat this as a general
exclusive-sharing guarantee for buffers or compute image handoffs.
A 32-second Debug opt-in run completed 19,603 frames; private bytes at
10/20/30 seconds were 387,371,008 / 387,371,008 / 387,330,048. This bounded
sample does not prove indefinite memory stability.
For the new storage-buffer handoff path, set
`GENGINE_VULKAN_EXCLUSIVE_COMPUTE_BUFFERS=1` together with
`GENGINE_VULKAN_DEDICATED_QUEUES=1`. The existing ColorData graph buffer is
read on graphics, written on compute, then read on graphics. Expect the
frame-119 numeric compute readback, no queue-family validation errors, and
normal shutdown. A second buffer starts with compute as its first graph use,
then drives a graphics draw and separate numeric readback; expect the
`Compute-first GPU graph readback passed` message too.
The 2026-10-19 VS2022 Debug/Release runs passed 120 frames and all three
readbacks on OpenGL, D3D12, Vulkan shared, and Vulkan dedicated queues. The
dedicated mode selected graphics=0, compute=2, transfer=5; buffer-only and
buffer-plus-image opt-ins passed without captured validation messages. A Debug
10,000-frame both-opt-in run also passed. For a bounded memory control, disable
replacements with `GENGINE_EXAMPLE_NO_REPLACEMENT=1`: the 32-second both-opt-in
run stayed near 412.4 MB private bytes at 10/20/30 seconds. With replacement
enabled, both opt-in and concurrent-sharing runs rose by about 4 MB over that
interval; investigate the replacement path separately.
The 2026-10-18 final VS2022 Debug and Release runs completed 120 frames on
OpenGL, D3D12, Vulkan shared, and Vulkan dedicated queues. Each passed the
compute and array-layer pixel readbacks and exited normally with empty stderr;
no D3D12 Debug Layer or captured Vulkan validation errors appeared. The
dedicated Vulkan run selected graphics=0, compute=2, transfer=5. D3D12
synthetic-resize runs in both configurations also logged 960x540 and
1280x720 recreations with no Debug Layer issues. This is a bounded
noninteractive check, not proof of long-run memory stability or visible
window behavior.
Debug D3D12 and Vulkan dedicated 10,000-frame follow-ups also passed both
readbacks and normal shutdown without captured validation issues. A 30-second
Vulkan dedicated run reached 21,139 frames; private bytes at 10/20/30 seconds
were 414,900,224 / 414,900,224 / 414,826,496. This bounded sample does not
close longer-term memory TODOs.

Startup additionally checks versioned producer/consumer ordering and stale-version
rejection after Reset, and creates the material/pipeline through RenderDevice.
Alternatively set `GENGINE_EXAMPLE_SECONDS` to a positive duration for a timed
replacement test that still exits normally. If both limits are set, the first
limit reached ends the run. Startup also checks overwrite-after-read ordering,
branching-write rejection and same-state shader-write dependency emission.

Startup also verifies subresource metadata and rejection paths: an empty range,
an overflowing range, an out-of-bounds transient texture range, and a non-whole
range on a resource without metadata must fail before allocation; a transient
two-mip texture must resolve `All`; a transient two-layer array, an imported
two-mip texture, and an imported two-layer texture array must accept valid
mip/layer ranges. It also checks untouched-mip state isolation and conservative
disjoint-range ordering.

The live GPU graph copies a red pixel from a 2D texture into layer 0 of a
two-layer RGBA8 texture array. A targetless graphics pass declares the source
as `CopySource` and only layer 0 as `CopyDestination`; a later pass returns both
layers to `ShaderRead`. At frame 119, `ReadLayerData` must report red for layer
0 and the original green for layer 1, logging
`GPU graph texture-array layer readback passed after 119 frames.` A mismatch
throws. Every copy pass also checks that an invalid layer, mismatched size,
and mismatched format are rejected before the valid copy. The triangle remains the expected alternating orange/green visible
result; the copied pixel is checked numerically, not shown on screen. Run each
backend with a frame limit of at least 120 and check this log, the compute
readback log, normal shutdown, and backend validation/debug output. Repeat
with resize, resource replacement enabled, and manual close for lifecycle
checks; interactive resize and long-run memory behavior are not established by
the bounded run. OpenGL uses whole-resource memory barriers and conservative
graph ordering even though the copy addresses one layer.

Texture-copy verification (2026-10-03, VS2022 x64): Debug and Release engine
and example builds passed. OpenGL, D3D12 with the debug layer, Vulkan shared
queues, and Vulkan dedicated queues each completed 120 frames, passed both
frame-119 numerical readbacks, and exited normally in both configurations.
The Vulkan dedicated run selected graphics=0, compute=2, transfer=5. No Vulkan
validation errors or D3D12 debug errors/warnings appeared in those logs.
Manual visible output, resize/minimize, and long-run memory stability were not
checked in this bounded verification.

Noninteractive lifecycle verification (2026-10-11, existing VS2022 Debug
binary): 120-second timed runs on OpenGL 4.6, D3D12 12.2 with and without its
debug layer, and Vulkan 1.4.312 with dedicated queues all passed the frame-119
compute and texture-array readbacks and exited normally. OpenGL private bytes
were about 173 MB at 10 and 120 seconds; dedicated Vulkan rose from about
415 MB to 420 MB near the end. D3D12 without the debug layer stayed near
171 MB, whereas debug-layer runs rose from about 192 MB to 210-212 MB even
with periodic resource replacement disabled. This is process memory, not
proof of an engine leak or its absence. The D3D12 debug layer emitted only
the known initial-UAV-state warning; no backend logged a validation error.
Isolated configuration copies preserved the user's `Config.ini`. No desktop
input was used, so visible output, real resize, and minimize/restore remain
untested. Detailed samples and the D3D12 diagnosis TODO are in
[cross-computer integration](../../TODO/CrossComputerIntegration.md).

Range verification (2026-09-29): VS2026 Debug and Release builds passed with
MSVC 14.50.35717. OpenGL 4.6 (NVIDIA driver 610.88) completed 120 frames,
passed frame-119 compute readback, and exited with code 0 in both Debug and
Release. D3D12 feature level 12.2 did the same in both configurations. Vulkan
1.4.341 did the same with shared queues and dedicated queues (graphics=0,
compute=2, transfer=4) in both configurations. An initial D3D12/Vulkan
pre-first-frame stop was traced to presentation wrappers lacking subresource
metadata; exposing their one-mip/one-layer swap-chain images fixed it. Vulkan
validation layers were unavailable on this host, so those runs had no
validation-layer coverage.
These runs include the exact-range transition, untouched-mip state, and
conservative disjoint-range dependency regressions.

Range GPU verification (2026-10-03): VS2022 Debug and Release x64 builds of
GEngine and FrameGraphTriangle passed. The two-layer GPU transition passes ran
for 120 frames on OpenGL 4.6, D3D12 12.2, and Vulkan 1.4.312 with shared and
dedicated queues; all runs passed frame-119 readback and exited normally.
Vulkan Debug validation initially reported an unsupported graphics source stage
on the dedicated compute queue. After constraining barrier stages to the
recording queue, Debug shared and dedicated runs reported zero validation
errors. The dedicated run selected graphics=0, compute=2, transfer=5.

OpenGL startup regression: select `GraphicsAPI: 1`; startup must reach the
OpenGL vendor/version logs and the triangle instead of crashing at the first
`glEnable`. Default GL state is initialized only after a current context and
successful GLAD loading. Then exercise the same replacement/resize/close checks.

OpenGL verification (2026-09-12): after fixing initialization, Vulkan-SPIR-V
translation to GLSL, VAO creation/deletion, attribute component counts and the
optional index-buffer bind, Debug built successfully. A 30-second OpenGL 4.6
run had no GL_INVALID errors, assertions or crashes; private bytes were
169664512 / 169664512 / 169684992. NVIDIA emitted shader-recompilation performance
warnings. The process was stopped at the time limit; visible output, resize,
normal shutdown, long-run memory and textured/complex shaders remain unverified.

Startup requests the same shader 64 times, alternating the original path with a
`./` alias, and checks object identity. Every 30-frame material replacement also
calls `Shader::Create` again. Check that shader loading/reflection messages occur
only for the initial load, not per request: identity alone would not detect the
old implementation constructing and discarding a duplicate before cache lookup.

Factory-cache verification (2026-09-12): Debug build passed; a 30-second Vulkan
run logged exactly one initial shader load and no validation errors. Private
bytes at 10/20/30 seconds were 401768448 / 401768448 / 401772544. The bounded
process was stopped, so normal shutdown, resize, cache-clear/reload, long-term
stability and other backend runs remain unverified.

Startup checks also cover reverse access declaration order, repeated graph
compilation, dependency-cycle detection, transient read-before-write, and invalid
render-target usage. One expected cycle diagnostic is printed by the negative
test. Failure throws before drawing; these checks do not validate GPU barriers.

The triangle alternates left/right every 30 rendered frames while replacing its
pipeline, material, and vertex buffer. Each replacement allocates new Vulkan
descriptor sets; old sets must be reclaimed after submission completion. The
shader is shared through the shader cache, so this tests descriptor-set turnover
but not repeated shader-layout replacement. Run for several minutes, resize the window, then
close it. Check for stable memory and clean backend validation output. Test each
API independently. Startup also checks implicit RenderGraph compilation without
assertions, so run both Debug and Release. Build success alone is not a runtime
validation result.

The non-instanced replacement path also checks optional Vulkan allocation
lifetime: no instance buffer is allocated, and retiring the vertex buffer must
not free an uninitialized instance-memory handle. Check that repeated movement
and shutdown produce no `vkFreeMemory` validation errors.

On Vulkan, also watch for `vkFreeDescriptorSets` in-use errors, descriptor-pool
exhaustion, and leaked device children at shutdown. Shader layouts are released
when the cache is cleared during shutdown. Repeated shader-layout replacement
and independent compute/transfer stress remain separate, unverified scenarios.

On D3D12, launch with `GENGINE_D3D12_DEBUG_LAYER=1` in the process environment.
Repeated replacement must not log `ID3D12PipelineState::<final-release>` while
GPU work is in flight, allocator-reset errors, or fail at swapchain `Present`.
The expected Producer-cycle diagnostic is now preceded by an explicit negative
test message. It is unrelated to presentation or GPU lifetime errors.

D3D12 fix verification (2026-09-12): Debug build passed. With the debug layer
enabled, a 30-second replacement run produced no D3D12 errors or warnings;
the previously reproduced PSO final-release corruption did not recur. Private
bytes at 10/20/30 seconds: 184463360 / 185028608 / 185348096. The process was
stopped at the time limit. Normal shutdown, resize and long-run memory stability
are not established by this check.

Verification (2026-09-12): Debug engine/example build passed. A 30-second Vulkan
smoke run logged no validation errors; private bytes at 10/20/30 seconds were
404291584 / 404422656 / 404422656. The process was stopped after the bounded test,
so this does not verify normal shutdown, resize, visible output, or long-duration
stability. OpenGL, D3D12, and Release were not run in this check.
