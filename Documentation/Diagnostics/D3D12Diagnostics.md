# D3D12 diagnostics and troubleshooting

## Enable validation

The D3D12 debug layer is opt-in in Debug builds. Enable it for a diagnostic run:

```bat
set GENGINE_D3D12_DEBUG_LAYER=1
Triangle.exe
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
allocations. A later opt-in Debug shutdown report compared 120 and 60,000
frames after application and renderer teardown. Both live-object summaries
contained identical external object-category counts, including 3 queues,
26 fences, 27 allocators, 25 command lists, 11 resources, and 5 pipeline
states. Context-owned objects are still alive at this report point. In the
60,000-frame layer-on run, private bytes rose from 193,826,816 at 10 seconds
to 209,403,904 at 70 seconds; a layer-off 60,000-frame control stayed near
171 MB, and an 80-second layer-off control stayed near 169 MB. Both readbacks,
normal shutdown, and empty stderr were observed in each run. The only layer-on
warning was the existing initial UAV buffer-state warning. This excludes
growth in the reported D3D12 object counts, not CPU allocations inside the
engine, validation runtime, or driver. Native heap allocation stacks remain
necessary to assign ownership and require an explicitly scoped elevated
trace; no machine-level heap tracing was enabled in this check.
An attempt to enable WPR heap tracing from the current non-administrator
session failed with `0x80070005 Access is denied`; subsequent checks showed
the per-process flag disabled and WPR idle. To collect the missing evidence,
use an administrator terminal after confirming `wpr -status` is idle. Enable
`wpr -HeapTracingConfig FrameGraphTriangle.exe enable`, then start
`wpr -start Heap -filemode`. Run a bounded Debug D3D12 FrameGraphTriangle
scenario with `GENGINE_D3D12_DEBUG_LAYER=1`, stop the recording to an ETL via
`wpr -stop <chosen-output.etl>`, and disable the flag with
`wpr -HeapTracingConfig FrameGraphTriangle.exe disable`. Repeat from a clean
recording with the layer set to `0` for a control. If a run fails, cancel any
active WPR session and disable the process flag before leaving the elevated
terminal. The ETL can contain system-wide process metadata; inspect sharing
scope before distributing it. Compare the target process's surviving heap
allocation stacks and modules, not only total private bytes.
The user authorized and approved an elevated capture on 2026-10-18. Two
60-second ETLs were saved under `tmp/d3d12-wpr-20261018/` and the helper
confirmed WPR idle and heap tracing disabled afterward. The layer-on/off
examples reached 16,973/37,203 frames, both passed the two GPU readbacks and
normal shutdown, and both had empty stderr. With WPR active, layer-on private
bytes rose from 184,946,688 at 10 s to 188,690,432 at 55 s; layer-off stayed
near 169,242,624. `xperf -a heap` reported 3,393.5 KiB versus 275.8 KiB
outstanding at the end of the 10-60 s event window. The layer-on stack report
contains 1,964.3 KiB through `nvwgf2umx.dll`, and 169.9 KiB from 10,876
allocations entering the NVIDIA driver and SDK layer via
`D3D12CommandBuffer::ResetRecording`. A 525.4-KiB group is only symbolized
as `ntdll` and remains unattributed. These are range-end
heap allocations, not a complete accounting of private bytes or proof of a
permanent leak. The most likely identified source is validation/driver CPU
bookkeeping on repeated command recording; exact ownership of the
`ntdll`-only group and the full private-byte increase remains open.
Across the full layer-on trace, outstanding heap allocations rose from
22,585.0 KiB at 10 s to 25,003.9 KiB at 60 s; a repeated NVIDIA-driver
stack accounted for 2,335.9 KiB at 60 s. This does not establish whether
the runtime/driver intentionally retains debug bookkeeping or leaks it.

The initial D3D12 storage-buffer UAV-state warning was a separate engine
state bug. Buffers now start in COMMON and complete an explicit transition
to UAV before `SetData` and graph use. Debug/Release builds and 120-frame
readbacks passed; the Debug Layer no longer reports that warning. In an
unblocked 60-second Debug run, the layer-on process still rose from
193,081,344 to 205,504,512 private bytes (10 to 60 s, 48,424 frames),
while the layer-off process stayed near 170.5 MB (86,880 frames). Thus the
state fix did not remove the validation-associated CPU memory trend. An
earlier run with an unread redirected stdout pipe was output-blocked at
348 frames and is not used as memory evidence.

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
