# Graphics API usage

This page describes the common immediate rendering path used by `Example/Triangle`. Include `<GEngine.h>` in an application project.

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

## Shader asset

Shaders use the `.shader` container format. A `Pass` contains fixed render state, stage pragmas and HLSL-like program code.

```hlsl
Shader "Triangle"
{
    Pass
    {
        Cull Off
        DepthTest Always
        DepthWrite Off
        ColorMask R G B A

        #pragma vertex vert
        #pragma fragment frag

        Program
        {
            struct VsInput { float3 position : POSITION; };
            struct VsOutput { float4 position : SV_POSITION; };

            VsOutput vert(VsInput input)
            {
                VsOutput output;
                output.position = float4(input.position, 1.0f);
                return output;
            }

            float4 frag(VsOutput input) : SV_Target
            {
                return float4(0.15f, 0.75f, 1.0f, 1.0f);
            }
        }
    }
}
```

Vertex-input semantics and types must match the memory supplied through the vertex buffer. In the example, `float3 position : POSITION` consumes 12 bytes per vertex. DX12 reflection maps this to `DXGI_FORMAT_R32G32B32_FLOAT`.

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

## Resources

- `Texture2D::Create`, `CubeMap::Create`, and texture-array APIs create sampled image resources.
- `UniformBuffer` and `Material` hold shader properties and constant-buffer data.
- `Sampler` and combined texture/sampler types provide sampling state.
- `StorageBuffer`, `StorageImage2D`, and `ComputePipeline` provide compute resources and dispatch.
- `FrameBuffer` and `RenderPass` create off-screen rendering targets.

Use `Material::Set*` APIs that correspond to reflected shader property names. Recreate or update the material's resources after changing the shader interface.

## Shader cache

Compiled shader data is stored under the directory configured by `ShaderCacheDirectory` in `Config.ini`. The cache is keyed by source hash and backend. Reflection still runs when cached DXIL/SPIR-V is loaded, so engine-side reflection fixes do not require editing the shader source.
