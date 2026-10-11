# D3D12 diagnostics and troubleshooting

## Enable validation

The D3D12 debug layer is opt-in in Debug builds. Enable it for a diagnostic run:

```powershell
$env:GENGINE_D3D12_DEBUG_LAYER = '1'
.\Triangle.exe
```

The engine enables the D3D12 debug layer before device creation and drains warnings/errors from a bounded info queue. The queue deliberately filters informational messages and has a finite capacity so validation cannot grow memory without limit during a render loop.

For a memory-isolation run only, set
`GENGINE_D3D12_INFO_QUEUE_LOGGER=0` alongside the Debug Layer switch. This
keeps validation and the 1,024-message queue limit but skips the polling
logger; warnings/errors are not streamed to the engine log in this mode.
Unset the override for the normal diagnostic run. Neither mode alone proves
whether process-private growth belongs to the engine, validation runtime, or
driver.
For a bounded engine-retirement check, `GENGINE_D3D12_LIFETIME_TRACE=1` logs
pipeline creation/live counts and, every 10,000 tracked submissions, the
completed serial, pending submissions, deferred releases, and stored/discarded
info-queue messages. A flat count cannot identify allocations retained inside
the D3D12 runtime or driver; compare it with process and GPU memory samples.

On 2026-10-11, matched 60,000-frame FrameGraphTriangle runs passed both GPU
readbacks and normal shutdown. With the Debug Layer off, private bytes stayed
near 173.5 MB. With it on, they rose by about 16 MB over roughly 100 seconds;
the rise persisted with `GENGINE_D3D12_INFO_QUEUE_LOGGER=0`. Sampled GPU
dedicated/shared usage stayed flat, and the existing pipeline lifetime trace
reported two live pipelines after 2,000 creations. This narrows the issue to
validation-enabled CPU-side behavior, but is not a native allocation trace or
proof of a driver leak. Keep the ownership investigation open in
[CrossComputerIntegration.md](../../TODO/CrossComputerIntegration.md).
An additional 60,000-frame Debug Layer-on run with the context trace passed
both readbacks and normal shutdown. Pending tracked submissions stayed at
1-10, deferred releases and stored/discarded messages at zero across the
10,000-360,000-submission samples, while private bytes increased from about
193 MB to 207 MB and sampled GPU memory stayed flat. These bounded counters
rule out accumulation in those sampled engine queues, not unobserved CPU
allocations; native allocation stacks and a timed live-object comparison are
still needed to assign ownership.

The same change built in Debug and Release; a 120-frame Release D3D12 run
passed both readbacks and normal shutdown. The logger switch affects only
Debug builds.

Do not enable it for ordinary profiling; validation changes timing and may report known third-party messages.

## RenderDoc capture checklist

For an empty draw, inspect the pipeline in this order:

1. **Event list:** confirm a `DrawInstanced` or `DrawIndexedInstanced` event exists.
2. **Input Assembler:** verify the vertex-buffer address, stride, semantic names and DXGI format.
3. **Vertex output:** check clip-space position and `w`. A visible triangle normally has `w = 1` and x/y values within the clip volume.
4. **Rasterizer:** confirm culling, scissor and viewport. For a baseline triangle use `Cull Off`.
5. **Depth/stencil and blend:** use `DepthTest Always`, `DepthWrite Off`, full `ColorMask`, and no blending to isolate output.
6. **Output Merger:** verify the bound RTV is the current swap-chain back buffer and the target was transitioned to render-target state.
7. **Final resource state:** confirm it returns to Present before `IDXGISwapChain::Present`.

## Common symptoms

| Symptom | Likely cause | First check |
| --- | --- | --- |
| `float3` appears as multiple scalar vertices | Reflection derived an incorrect input format/stride. | Vertex signature Mask and generated `DXGI_FORMAT`. |
| Correct VS positions but no fragments | Face culling, depth/stencil, scissor, or color-mask state removes output. | Use `Cull Off`, `DepthTest Always`, full color mask. |
| Blank window with no diagnostic | `Present` failure was ignored or no draw is recorded. | Check `Present` HRESULT and RenderDoc event list. |
| Memory grows every frame | Command allocators or descriptors are not reused/freed after their fences complete. | Reset the per-frame allocator only after waiting for its fence. |
| `GEngine.pch` is locked | Two builds share the intermediate directory. | Stop the duplicate build; use one build invocation. |
| `vc143.pdb` C1041 | Parallel compilers write a shared PDB without `/FS`. | Regenerate with the updated Premake setting or ensure `/FS` is present. |

## D3D12 lifetime rules

- A command list may be reset only after it has been closed.
- Its allocator may be reset only after the GPU has completed commands recorded with that allocator.
- Keep shader bytecode alive for as long as a PSO uses its `D3D12_SHADER_BYTECODE` pointers.
- Keep descriptor-range arrays alive until `D3D12SerializeRootSignature` finishes.
- Check HRESULT values from device, swap-chain, command-list and resource-creation calls; do not discard failures from `Present`.

## Triangle baseline

`Example/Triangle` is the intended first D3D12 smoke test. Its shader consumes one `POSITION` `float3`, writes an opaque cyan `SV_Target`, disables culling via `Cull Off`, uses an always-pass depth test and does not write depth. If this sample fails, fix the rendering foundation before debugging higher-level scene rendering.
