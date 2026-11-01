## Off-screen command submission

Camera stacking adds two offscreen command operations (2026-11-01, verified by
P2 Debug/Release runs on GL/Vulkan/D3D12). `BeginRenderPass(framebuffer, renderPass)` selects a device-created
compatible pass: only load/store operations may differ from the framebuffer's
pass. Attachments/sample count/depth resolve/subpasses must match. Commands retain
the pass and framebuffer through native submission; shared framebuffer operations
are not mutated. The original one-argument overload remains supported.

`SetViewport(GraphicsViewport{X,Y,Width,Height})` sets viewport plus matching
scissor in top-left target pixels. An active graphics target and nonempty,
in-bounds rectangle are required; overflow/out-of-bounds calls throw in both
builds. Begin resets the command's viewport to the whole target. Clears and MSAA
resolve cover whole attachments, independent of prior camera scissor. Vulkan draws
honor the recorded viewport, GL translates its Y origin and clears/resolves reset
scissor; presentation starts from full-window state. Rebuild apps/backends with
the engine for the new virtual command/framebuffer overloads.

Use `OnRender` for off-screen work. Record a command buffer first and submit it
through the queue matching its type. Queue selection stays portable across all
three backends:

```cpp
auto commandBuffer = GEngine::Graphics::GetGraphicsCommandBuffer();
commandBuffer->Begin(frameBuffer);
commandBuffer->Render(m_Pipeline, 0);
commandBuffer->End();

auto& queue = GEngine::Graphics::GetRenderDevice().GetQueue(
    GEngine::COMMAND_BUFFER_TYPE_GRAPHICS);
queue.Submit(commandBuffer);
```

Do not submit a presentation command buffer manually. `GraphicsPresent` owns
swap-chain acquisition, presentation synchronization, and its final submit.

D3D12 storage-buffer/image uses recorded by resource barriers or material binding
retain the resource and carry their last submitted completion across command
buffers and frames. A consumer on another native queue waits on that exact fence
value; recording alone does not advance it. Existing RenderGraph declarations
and `SetCommandsBarrier` edges remain required for graph ordering. No new public
method or native handle is exposed; the fix supplements those edges for UAV
resources shared between consecutive executions. Other backends are unchanged.
