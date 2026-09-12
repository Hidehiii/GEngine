# FrameGraphTriangle

This is a visual smoke test for the portable presentation path.  It renders an
orange triangle through only public engine APIs; it does not include D3D12,
Vulkan, or OpenGL headers and it does not access native handles.

The layer submits one off-screen graphics command buffer through the public
`GraphicsQueue`, then makes the presentation command buffer depend on it. The
engine's `RenderSystem` builds the following per-frame graph around the layer
draw:

```text
LayerRender (off-screen triangle, queue submit)
    -> Present (visible triangle draw) -> ReleasePresentation
                                      Present -> RenderTarget -> Present
```

Run the project after selecting the desired `GraphicsAPI` in `Config.ini`.
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

## Resource replacement check

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
