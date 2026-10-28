## Off-screen command submission

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
