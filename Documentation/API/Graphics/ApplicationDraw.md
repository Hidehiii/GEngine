## Minimal draw

```cpp
void TriangleLayer::OnAttach()
{
    const float vertices[] =
    {
         0.0f,  0.65f, 0.0f,
         0.65f, -0.55f, 0.0f,
        -0.65f, -0.55f, 0.0f
    };

    auto shader = GEngine::Shader::Create("Assets/Shaders/Triangle.shader");
    auto material = GEngine::Material::Create(shader, "TriangleMaterial");
    auto vertexBuffer = GEngine::VertexBuffer::Create(vertices, sizeof(vertices));
    m_Pipeline = GEngine::GraphicsPipeline::Create(material, vertexBuffer);
}

void TriangleLayer::OnPresent()
{
    GEngine::GraphicsPresent::Render(m_Pipeline, 0);
}
```

`GraphicsPresent::Render` selects the current presentation command buffer. With no index buffer, the pipeline derives the vertex count from the reflected per-vertex stride. Supply an index buffer through `VertexBuffer::SetIndexBuffer` to use indexed drawing.
