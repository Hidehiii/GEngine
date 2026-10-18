# Graphics and compute API

[API index](README.md) · [Rendering architecture](../Architecture/Rendering.md) ·
[Render graph API](RenderGraph.md)

This page covers the backend-neutral graphics/compute resource and command
surface plus a minimal draw. Runtime, scene, and service entry points are in
their own function guides. Public headers are authoritative for signatures;
planned features remain in TODO and must not be described as implemented.
Follow the [Standard workflow](../Workflow/StandardWorkflow.md) when changing
this API or its architecture.

## Diagnostic storage-buffer readback

`StorageBuffer::ReadData(size, destination, offset = 0)` copies bytes to CPU
memory synchronously and validates bounds in Debug and Release. The buffer must
be in storage/UAV state. D3D12 and Vulkan wait for submitted queues, stage a copy,
and wait for that copy before reading; OpenGL uses its buffer-update barrier and
synchronous readback. This is for diagnostics and infrequent results, not
per-frame streaming. Call it after submitting producers, outside recording
callbacks. FrameGraphTriangle checks an actual compute result once.

## Device-owned creation

`Graphics::GetRenderDevice()` now creates shaders, materials, graphics/compute
pipelines, samplers, storage buffers/images, 2D/array/cube textures and combined
texture/sampler wrappers, in addition to render passes, framebuffers and mesh
buffers. Native backend selection lives in backend implementations, not these
resources' static factory functions.

```cpp
auto& device = Graphics::GetRenderDevice();
auto shader = Shader::Create("Assets/Shaders/FrameGraphTriangle.shader");
auto material = device.CreateMaterial(shader, "Triangle");
auto pipeline = device.CreateGraphicsPipeline(material, vertices);
```

`device.CreateShader(path)` creates an uncached backend object;
`Shader::Create(path)` retains the existing runtime-scoped cache behavior.
Similarly, `Sampler::Create` caches while `device.CreateSampler` allocates a new
sampler. Other static factories remain compatibility adapters. This does not yet
support multiple simultaneously active devices or cross-device resource mixing.

`Material::GetShader()` now returns `Ref<Shader>` by value. The previous reference
return exposed a temporary shared_ptr on OpenGL/Vulkan; callers should hold the
returned shared_ptr rather than binding a mutable reference to it.

`RenderDevice` also owns creation of render passes/framebuffers and mesh
buffers. `GraphicsQueue::Submit` accepts a completed command buffer of its
queue type; `WaitForIdle` is for shutdown or exceptional diagnostics, not a
normal per-frame dependency. `GraphicsPresent` owns swap-chain acquisition and
submission; do not submit its command buffer yourself. OpenGL, Vulkan, and
D3D12 are separate implementations of these common contracts, not three
interchangeable native-handle types.

`RenderSystem::Resize(width, height)` forwards nonzero sizes to the presenter's
`Resize` hook after updating the viewport. D3D12 rebuilds its swap-chain
targets and presenter command lists when dimensions change; the default hook
does nothing for OpenGL and Vulkan, which retain their existing resize paths.
The runtime skips zero-size events. This is a lifecycle hook, not an
application-level resize API; the user reported the interactive D3D12 check
passed, while detailed debug/memory measurements remain open.
The hook does not expose D3D12 fence handles to application code.
Example-only diagnostic: `GENGINE_EXAMPLE_SYNTHETIC_RESIZE=1` makes
FrameGraphTriangle inject 960x540 and 1280x720 resize events after frames 30
and 60 on D3D12. It does not change the native window, add a production
graphics API, or prove interactive resize behavior.
Both diagnostic D3D12 resize events completed in Debug and Release with the
same frame-119 readbacks. The real window resize/minimize check was reported
successful by the user, not reproduced by the agent.

No public graphics signature changes for D3D12 diagnostic isolation. In a
Debug D3D12 process, `GENGINE_D3D12_DEBUG_LAYER=1` opts into validation;
`GENGINE_D3D12_INFO_QUEUE_LOGGER=0` separately suppresses the polling logger
for memory comparisons without disabling the layer. Keep the logger enabled
for runs where warning/error output is required; see
[D3D12 diagnostics](../Diagnostics/D3D12Diagnostics.md).
`GENGINE_D3D12_LIFETIME_TRACE=1` is also diagnostic-only: it records pipeline
and context-retirement counts without adding or changing a public API.

For scheduled GPU work, `RenderGraph` imports resources or declares transient
texture-array, 2D texture, storage-buffer, and storage-image descriptions.
`BuildGraphicsPass` records into a target, `BuildGraphicsCopyPass` records a
targetless graphics-queue copy, and `BuildComputePass` records dispatch.
Declare `Read`/`Write` states and ranges before `ExecuteGpu`; use versioned
resources to order producers and consumers. See the detailed
[render-graph API](RenderGraph.md). The common device method
`CopyTexture2DToArrayLayer(command, source, destination, layer)` validates
matching size/format and the destination layer. `Texture2DArray::ReadLayerData`
is an exact-size RGBA8 synchronous diagnostic readback, not streaming I/O.

## Device capabilities

`Graphics::GetCapabilities()` returns a backend-specific snapshot derived from
device queries rather than API identity. The result includes the backend name,
queried version and the concrete query sources used to derive the feature
booleans.

```cpp
const auto capabilities = Graphics::GetCapabilities();
GE_INFO("Backend {} version {}.", capabilities.Backend, capabilities.Version);
for (const auto& source : capabilities.QuerySources)
    GE_INFO("Capability source: {}.", source);
```

OpenGL uses its core version and enumerated extensions. Vulkan queries physical
device properties/features, device extensions, format properties and queue
families. D3D12 queries feature levels, options, root-signature and shader-model
support plus the format support used by render targets and storage images.
Capability reporting stays per-device; one backend never infers support from
another.

The graphics sections below describe the common immediate rendering path used
by `Example/Triangle`. Include `<GEngine.h>` in an application project, and add
specific headers for APIs outside that umbrella.

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

## Resources

- `Texture2D::Create`, `CubeMap::Create`, and texture-array APIs create sampled image resources.
- `UniformBuffer` and `Material` hold shader properties and constant-buffer data.
- `Sampler` and combined texture/sampler types provide sampling state.
- `StorageBuffer`, `StorageImage2D`, and `ComputePipeline` provide compute resources and dispatch.
- `FrameBuffer` and `RenderPass` create off-screen rendering targets.

Use `Material::Set*` APIs that correspond to reflected shader property names. Recreate or update the material's resources after changing the shader interface.

## Shader cache

Compiled shader data is stored under the directory configured by `ShaderCacheDirectory` in `Config.ini`. The cache is keyed by source hash and backend. Reflection still runs when cached DXIL/SPIR-V is loaded, so engine-side reflection fixes do not require editing the shader source.
