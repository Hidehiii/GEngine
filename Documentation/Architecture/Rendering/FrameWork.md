## Frame work and graph

`RenderSystem` owns the presentation graph and `FrameContext`. Per frame it
acquires the swap-chain image, records `LayerRender` (`Layer::OnRender`), an
optional ImGui pass, and `Present` (`Layer::OnPresent`), transitions the
presentation image to a render target and back, then submits. `RenderGraph`
expresses pass ordering, resource versions, resource states, and subresource
ranges. D3D12 records resource/UAV barriers, Vulkan records image/buffer/memory
barriers, and OpenGL emits memory barriers without tracking image layouts.
The graph validates state/read-write and state/stage compatibility when an
access is declared, then checks the declared stage against the pass queue
during compilation. Write-only attachment/copy/presentation states cannot be
declared as reads, read-only shader/copy-source states cannot be declared as
writes, and copy states require the transfer stage. Vulkan derives native
access masks only from compatible declaration intent and removes compute-shader
scope from graphics command barriers. Validation runs in both Debug and
Release before transient allocation or command recording.

Renderer features can use a persistent GPU graph with `BuildGraphicsPass`,
`BuildGraphicsCopyPass`, `BuildTransferCopyPass`, `BuildComputePass`, graph-owned
targets, and `ExecuteGpu`. The graph owns command Begin/End, attachment binding,
dependency submission, and the presentation join. Versioned resources order
producers, readers, and overwrites independently of declaration order. CPU-only
`BuildPass`/`Execute` remains a compatibility path, not cross-queue GPU
synchronization. Disjoint texture ranges are capability-gated: D3D12 and
Vulkan can omit false inferred dependencies when per-mip/per-layer state is
tracked, while OpenGL and resources without metadata remain conservative. See
[RenderGraph.md](../../API/RenderGraph.md) for the exact contract.

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
The 2026-10-31 current-source recheck rebuilt CMake Debug/Release and repeated
the backend matrix after the backend-module split. OpenGL, D3D12, Vulkan
shared, and Vulkan dedicated queues passed all three readbacks and normal
shutdown; D3D12 used its debug layer without reported issues, and dedicated
Vulkan selected graphics=0, compute=2, transfer=4.
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
