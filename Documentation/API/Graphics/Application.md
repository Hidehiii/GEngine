## Application and layer

```cpp
class TriangleLayer final : public GEngine::Layer
{
public:
    TriangleLayer() : Layer("Triangle") {}

    void OnAttach() override;
    void OnPresent() override;

private:
    GEngine::Ref<GEngine::GraphicsPipeline> m_Pipeline;
};
```

Create the application with an `ApplicationSpecification`, then attach the layer with `PushLayer`. Use `OnPresent` for draws directed at the current swap-chain image. `OnRender` is available for off-screen or render-graph work that precedes presentation.
