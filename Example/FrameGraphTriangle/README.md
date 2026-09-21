# FrameGraphTriangle

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

## Resource replacement check

The merged example replaces materials, vertex buffers and graphics pipelines
every 30 frames. It also calls `SetRenderPassOperation` on the graph-owned
offscreen framebuffer to preserve the remote Vulkan render-pass retirement
regression. This runs alongside the graphics/compute graph and frame-119
numeric readback. `GENGINE_EXAMPLE_NO_REPLACEMENT` disables all these replacements
for baseline comparisons. Merged-version results are tracked separately in
[cross-computer integration](../../TODO/CrossComputerIntegration.md).

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
pipeline counts every 1,000 creations. Compare private bytes with
`GENGINE_D3D12_DEBUG_LAYER=1` and `0`; live object counts alone do not prove that
driver allocations are reclaimed.
Set `GENGINE_VULKAN_DEDICATED_QUEUES=1` for independent-family testing. Check the
selected-family log: a shared family is a compatibility run, not proof of
cross-family synchronization. The same frame-119 readback must pass.

Startup additionally checks versioned producer/consumer ordering and stale-version
rejection after Reset, and creates the material/pipeline through RenderDevice.
Alternatively set `GENGINE_EXAMPLE_SECONDS` to a positive duration for a timed
replacement test that still exits normally. If both limits are set, the first
limit reached ends the run. Startup also checks overwrite-after-read ordering,
branching-write rejection and same-state shader-write dependency emission.

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
