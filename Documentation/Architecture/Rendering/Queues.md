## Device queues and presentation

`RenderDevice` exposes portable `GraphicsQueue` instances for graphics,
compute, and transfer work. A `CommandBuffer` records with `Begin`/`End`;
submission is a separate queue operation. `Graphics::SubmitCommandBuffer`
remains a compatibility facade. `GraphicsQueue::WaitForIdle` is for shutdown
and exceptional synchronization, not routine per-frame lifetime management.

Presentation owns acquire, submit, fence, and present synchronization. On
Vulkan, inter-command dependencies live in the device submission layer;
the presenter uses one acquire semaphore per in-flight frame and one present
semaphore per swap-chain image. Recreating the swap-chain recreates those
presenter-owned semaphores after device idle, so a present semaphore is not
reused while its swap-chain image is still in use. Resize compares the
requested viewport with swap-chain extent before acquisition, avoiding an
abandoned signaled acquire semaphore. Debug messengers request only message
types supported by the enabled instance extensions, and device creation no
longer enables deprecated Vulkan device layers.
Vulkan allocations use concurrent sharing by default across distinct graphics,
compute, and transfer families; opt-in exclusive 2D/array transfer images use
ownership barriers as described above. Dependency semaphores provide queue
ordering. D3D12
dependencies identify a producer submission generation rather than a mutable
shared fence value. OpenGL executes the logical chain on one context.

D3D12 UAV buffers/images additionally retain their last submitted queue, fence
and monotonic value. Barriers and material bindings register resource use on the
recording command. Before submission, another queue waits on that recorded value;
after the command's signal succeeds, its exact completion replaces the resource's
last-use token. This covers separate graph executions and presentation material
uses, including graphics in frame N followed by compute in frame N+1. Same-queue
ordering stays FIFO. CPU uploads/readbacks wait on an outstanding other-queue use
before their existing one-time graphics submission.

Recording/discarding a command never publishes a completion token. Each command
retains the resource owning its borrowed usage record through GPU completion and
clears its deduplicated use list on safe allocator reset. Tokens own only native
fences, not command-buffer objects, so they cannot form an owner cycle. Existing
per-command fences are reused; there is no per-frame fence allocation, new CPU
queue-wide wait, or device-wide idle path. This is backend bookkeeping under the
existing public queue contract; OpenGL/Vulkan implementation and API are unchanged.

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
[RenderingArchitecture.md](../../../TODO/RenderingArchitecture.md).
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
[CrossComputerIntegration.md](../../../TODO/CrossComputerIntegration.md) for the
samples and the open diagnosis.
