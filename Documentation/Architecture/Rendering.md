# Rendering and backend architecture

[Architecture overview](Architecture.md) · [Graphics API](../API/GraphicsAPI.md) ·
[Render graph API](../API/RenderGraph.md)

## Frame work and graph

`RenderSystem` owns the presentation graph and `FrameContext`. Per frame it
acquires the swap-chain image, records `LayerRender` (`Layer::OnRender`), an
optional ImGui pass, and `Present` (`Layer::OnPresent`), transitions the
presentation image to a render target and back, then submits. `RenderGraph`
expresses pass ordering, resource versions, resource states, and subresource
ranges. D3D12 records resource/UAV barriers, Vulkan records image/buffer/memory
barriers, and OpenGL emits memory barriers without tracking image layouts.

Renderer features can use a persistent GPU graph with `BuildGraphicsPass`,
`BuildGraphicsCopyPass`, `BuildTransferCopyPass`, `BuildComputePass`, graph-owned
targets, and `ExecuteGpu`. The graph owns command Begin/End, attachment binding,
dependency submission, and the presentation join. Versioned resources order
producers, readers, and overwrites independently of declaration order. CPU-only
`BuildPass`/`Execute` remains a compatibility path, not cross-queue GPU
synchronization. Disjoint ranges are still conservatively ordered. See
[RenderGraph.md](../API/RenderGraph.md) for the exact contract.

A transfer pass validates transfer-stage CopySource/CopyDestination accesses.
The graph records state/layout preparation on a graphics command, submits it
before a D3D12 COPY or Vulkan transfer command, then joins the copy submission
to consumers and presentation. OpenGL uses a graphics command buffer on its
single context. D3D12 hands 2D textures and arrays to COPY in COMMON and
tracks COPY-queue decay back to COMMON, including untouched array layers.
Vulkan keeps transfer image layouts and uses concurrent sharing by default
when queue families differ; opt-in exclusive 2D/array images use explicit
graphics/transfer release-acquire barriers for this copy path. Accesses with unchanged logical copy states
still receive backend preparation/support checks. FrameGraphTriangle exercises
the real graphics -> transfer -> graphics texture-to-array copy. Debug and
Release runs on OpenGL, D3D12, Vulkan shared, and Vulkan dedicated queues
passed both GPU readbacks and normal shutdown; D3D12 Debug Layer emitted no
issues, and the dedicated Vulkan run selected graphics=0, compute=2,
transfer=5 with no captured validation messages. Other resource types and
long-run/interactive behavior remain unverified.
Debug D3D12 and Vulkan dedicated also completed 10,000-frame reuse checks
without captured validation issues. A 30-second Vulkan dedicated run stayed
near 414.9 MB private bytes from 10 to 30 seconds; this is bounded evidence,
not a leak-freedom claim.

An opt-in Vulkan implementation creates `Texture2D` and `Texture2DArray`
images with exclusive sharing when `GENGINE_VULKAN_EXCLUSIVE_TRANSFER_IMAGES=1`.
The graph records graphics release after copy-layout preparation, transfer
acquire/copy/release, and graphics acquire before a later reader. Ownership is
tracked per mip/layer; the existing graph semaphore edges order submissions.
Debug/Release 120-frame dedicated-family runs and a 10,000-frame Debug reuse
run passed both readbacks with no captured validation issues. Concurrent
sharing remains default; exclusive buffers and arbitrary compute/graphics
handoffs were outside that image-only milestone.
In a 32-second Debug dedicated-family opt-in run, private bytes sampled at
10/20/30 seconds stayed near 387.4 MB; this is bounded, not a general leak
guarantee.

The new opt-in `GENGINE_VULKAN_EXCLUSIVE_COMPUTE_BUFFERS=1` path creates
Vulkan storage buffers with exclusive sharing. A compute pass that accesses
one receives a graphics preparation submission, then compute acquire/dispatch/
release; later graphics access acquires the buffer. The graph preflights
unsupported exclusive resource/queue combinations before submission. Buffer
upload/readback require graphics ownership. When graphics and compute share a
queue family, no ownership preparation or native handoff is needed. This path is implemented but not
yet general-purpose; exclusive compute images remain out of scope.
FrameGraphTriangle exercises both graphics-first and compute-first buffer
handoffs with real draws and numeric readbacks. Debug/Release 120-frame runs
passed on OpenGL, D3D12, Vulkan shared, and dedicated queues, with no captured
validation errors. Debug dedicated Vulkan completed 10,000 frames with both
opt-ins. In 32-second Debug runs, private bytes rose with replacement both
with and without exclusive sharing, while no-replacement controls were near
flat. This points to replacement interaction, not a proven ownership leak;
its allocation origin and longer-term behavior remain open.

Transient `Texture2D`, texture-array, storage-buffer, and storage-image
descriptions are allocated at graph compilation, exposed during pass execution,
and released at reset. The graph does not yet pool or alias their memory before
resource lifetime analysis.

## Device queues and presentation

`RenderDevice` exposes portable `GraphicsQueue` instances for graphics,
compute, and transfer work. A `CommandBuffer` records with `Begin`/`End`;
submission is a separate queue operation. `Graphics::SubmitCommandBuffer`
remains a compatibility facade. `GraphicsQueue::WaitForIdle` is for shutdown
and exceptional synchronization, not routine per-frame lifetime management.

Presentation owns acquire, submit, fence, and present synchronization. On
Vulkan, inter-command dependencies live in the device submission layer;
the presenter consumes the acquire semaphore and produces the present
semaphore. Resize compares the requested viewport with swap-chain extent
before acquisition, avoiding an abandoned signaled acquire semaphore.
Vulkan allocations use concurrent sharing by default across distinct graphics,
compute, and transfer families; opt-in exclusive 2D/array transfer images use
ownership barriers as described above. Dependency semaphores provide queue
ordering. D3D12
dependencies identify a producer submission generation rather than a mutable
shared fence value. OpenGL executes the logical chain on one context.

The runtime pauses rendering on a zero-size window event and updates its
viewport on a nonzero event. OpenGL applies the viewport during presentation;
Vulkan compares the swap-chain extent and recreates it before acquisition.
The D3D12 presenter now handles nonzero size changes after the presentation
graph releases its borrowed back-buffer view. It signals and waits on each
D3D12 graphics, compute, and transfer queue fence, collects completed
retirements, then destroys its old command lists (which retain the
old framebuffers), then has the context release old back-buffer targets and
call `ResizeBuffers` before rebuilding targets and presenter command lists.
Zero-size events remain paused by the runtime. This is an exceptional
resize-time wait, not a per-frame synchronization change. On 2026-10-18 the
user reported no problem with the manual D3D12 resize/minimize/restore check;
the agent did not observe the window or capture those manual logs. Detailed
debug-output and memory evidence remains open in
[RenderingArchitecture.md](../../TODO/RenderingArchitecture.md).
An opt-in example event sequence requests two nonzero presentation sizes
between frames without manipulating the native window. It exercises buffer
recreation but cannot certify OS window events or minimize/restore.
Debug and Release D3D12 runs logged both successful buffer recreations and
passed both GPU readbacks after 120 frames. OpenGL and Vulkan default runs
remained healthy; their resize paths were not exercised in this check.

For lifecycle evidence, the example's timed normal-shutdown path can exercise
frame submission and resource replacement without input automation. A
2026-10-11 Debug run covered OpenGL, D3D12, and dedicated-queue Vulkan for
120 seconds each; both numerical GPU readbacks and normal shutdown passed.
This does not test visible pixels, a real window resize, or minimize/restore.
Process private bytes were near-flat for OpenGL and D3D12 without its debug
layer, but rose with the D3D12 debug layer even when replacement was disabled;
ownership of that increase remains undetermined. See
[CrossComputerIntegration.md](../../TODO/CrossComputerIntegration.md) for the
samples and the open diagnosis.

## Backend layering and shader ownership

Application and renderer code use `GEngine/include/GEngine` interfaces.
`GEngine/src/Platform` and `GEngine/include/Platform` contain OpenGL, Vulkan,
and D3D12 resource, command, context, and presentation implementations;
`Surface` contains GLFW and Win32 windows/input adapters. Native handles,
allocation policies, and synchronization remain behind those boundaries.

OpenGL backend construction selects an API without issuing GL calls. Window
creation makes the context current and loads GLAD; `GraphicsRuntime::Initialize`
then sets up debug output/default GL state. Loader failure throws before a GL
state call, including in Release. OpenGL translates the compiler's Vulkan
SPIR-V intermediate to desktop GLSL 450 using SPIRV-Cross; it does not submit
Vulkan binaries to `glShaderBinary`. Shader compile/link failures throw and
release partial programs. Both vertex-buffer constructors create a VAO and
destruction deletes it; non-indexed draws do not require an index buffer.
Complex texture/sampler combinations still need focused coverage.

`Shader` owns reflection data and compiled bytes. Reflected vertex inputs set
buffer format/offset/stride; D3D12 uses DXIL reflection and Vulkan uses SPIR-V
reflection. `Shader::Create(path)` checks a cache keyed by active API and
lexically normalized absolute path before backend construction. Relative
paths resolve against the working directory; symlinks/case variants are not
canonicalized. `GetShader(name)` is a compatibility lookup, and
`ShutdownCache` clears both caches while the context is alive. Cache lookup
is render-thread-only, runtime-scoped, and not hot reload.

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
[D3D12 diagnostics](../Diagnostics/D3D12Diagnostics.md).

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

## Extension points

- Put cross-backend concepts in `GEngine/include/GEngine` and portable
  orchestration in `GEngine/src/GEngine`.
- Add backend implementations only for native behavior; do not expose native
  handles through application/renderer interfaces.
- Declare resource access/state through `RenderGraph` rather than a layer's
  backend-specific barriers.
- Exercise substantial rendering work in a focused `Example` before editor
  integration.
