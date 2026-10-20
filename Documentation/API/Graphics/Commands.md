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
