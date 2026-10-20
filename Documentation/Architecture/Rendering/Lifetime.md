## GPU resource lifetime

Vulkan replacement uses monotonic submission serials. Graphics, compute, and
transfer submissions have completion fences; presentation has an additional
completion marker. A contiguous completion watermark retires resources,
descriptor sets, shader modules/layouts, and render passes. The descriptor
pool is context-owned and destroyed only after device wait and retirement
flush. Short-lived uploads remain synchronous. Callers retain owners until
recorded commands are submitted; unsubmitted commands are not tracked.

D3D12 has separate mechanisms: context submission tracking retires replaced
native buffers/pipelines across queues, while each command buffer retains
recording owners and uses its own monotonic fence value before allocator reset.
Presentation reuses the same reset path and has one command buffer/fence value
per swap-chain frame slot. This protects whole-object replacement, not in-place
mutation of a retained resource. Native command-list users still retain their
resources. The D3D12 debug layer is opt-in with
`GENGINE_D3D12_DEBUG_LAYER=1`; see
[D3D12 diagnostics](../../Diagnostics/D3D12Diagnostics.md).

For memory isolation, the optional polling info-queue logger can be disabled
with `GENGINE_D3D12_INFO_QUEUE_LOGGER=0` while keeping the Debug Layer, bounded
message queue, and storage filter active. The default logger remains enabled.
This diagnostic switch does not change resource ownership or the public
rendering interface.
With `GENGINE_D3D12_LIFETIME_TRACE=1` in Debug, the context also samples
submission completion, pending submissions, deferred releases, and bounded
info-queue message counts every 10,000 tracked submissions. This is diagnostic
observation only; the queue/fence ownership and retirement rules are unchanged.

In a matched 60,000-frame comparison, process private bytes grew with the
Debug Layer regardless of whether this logger ran, while GPU process memory
and the example's live graphics-pipeline count stayed approximately constant.
The evidence excludes this polling thread as the main source but does not yet
identify the validation/runtime CPU allocation owner.
The later opt-in context trace likewise showed bounded pending submissions
and no deferred releases or retained info-queue messages over 60,000 frames;
private bytes still rose. Native allocation attribution remains open.
The opt-in `GENGINE_D3D12_LIVE_OBJECTS=1` shutdown comparison samples the D3D12
Debug Layer's live-device-object summary after application and renderer
teardown but before the context's own member objects are destroyed. The
polling logger is stopped before the report, and available info-queue messages
are copied to the engine log. It is diagnostic evidence about count growth
across frame counts, not a final zero-live-object assertion or CPU
allocation-stack attribution.
In the 2026-10-18 comparison, 120 and 60,000 Debug Layer-on frames yielded
identical live-object summary counts at shutdown, despite private bytes rising
from about 194 MB at 10 seconds to 209 MB at 70 seconds in the long run.
Layer-off frame- and time-based controls remained near-flat. This rules out
growth in the reported external D3D12 object categories at that point, not
unreported CPU allocations inside validation, driver, or engine code. The
known initial UAV-state warning remained; no new validation errors appeared.
The next diagnostic attempts a user-authorized, bounded WPR heap trace of the
existing D3D12 example. It changes only machine-level profiler configuration
for that process name while capturing and is not an engine memory-lifetime
mechanism. The tracing flag must be disabled after recording.
The attempted enable in the current non-administrator session returned
`0x80070005 Access is denied`; WPR remained idle and the process tracing flag
remained disabled. Therefore no native allocation-stack evidence was collected
and ownership is still unresolved.
The user has authorized one UAC-approved, bounded capture helper; it is
external profiling work and does not change renderer ownership or scheduling.
The approved 60-second layer-on/off WPR runs produced heap traces. During
10-60 s, the layer-on trace had about 3.39 MiB of heap allocations outstanding
at the window end versus 0.28 MiB off. Identified stacks pass through the
NVIDIA user-mode driver, D3D12Core, and d3d12SDKLayers; one repeated
small-allocation stack enters via `D3D12CommandBuffer::ResetRecording`.
An additional 525.4-KiB stack group contains only `ntdll` frames; the larger
1,964.3-KiB group includes NVIDIA driver frames. This narrows
the likely source to validation/driver-side CPU bookkeeping during command
recording, but does not prove every private byte's owner or an engine leak.
WPR was stopped and the per-process heap flag disabled after capture.
D3D12 storage buffers are created in COMMON and explicitly transitioned to
UAV, completing a one-time graphics submission before consumers or the render
graph treat them as ShaderWrite. This corrects the startup state contract;
the validation/driver heap growth is a separate, still-open investigation.
The corrected 60-second on/off runs after this state fix passed readbacks;
private bytes still grew with validation on and stayed near-flat with it off.
