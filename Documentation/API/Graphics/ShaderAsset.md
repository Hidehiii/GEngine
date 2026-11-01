## Shader asset

Include GEngine/Graphics/ShaderContainer.h for CPU-only validation:

```cpp
auto parsed = GEngine::ParseShaderContainer(payload, "Assets/Shaders/Example.gasset");
const auto& pass = parsed.Passes.at(0);
// pass.State, pass.EntryPoints, pass.Program and pass.MeshWorkflow are owned values.
```

ShaderContainer has Name and Passes; ShaderContainerPass has RenderState State,
an entry-point map keyed by canonical lowercase stages, exact Program bytes and
MeshWorkflow. Input string_views need only remain valid during the call. Invalid
input throws std::invalid_argument containing source name, one-based line and
byte column; .gasset diagnostics refer to the decoded payload, not YAML envelope lines.
No Application/device/compiler/cache is needed. Shader::Create(path) signatures
remain unchanged; loading parses before disk-cache access or compilation. Its
existing in-memory cache still reuses a previously loaded path without hot reload.
See [grammar/error rules](../ShaderAuthoring.md#container-parser-contract).
Debug/Release engine/example builds and --parser rollback/cache/output checks
pass separately on OpenGL, Vulkan shared, Vulkan dedicated and D3D12. The
former system D3D12 validation-runtime handle leak is isolated separately;
current mandatory Agility 1.619.5 integration passes 30000-frame Debug/Release
handle checks. Shader signatures/cache behavior are unchanged by SDK selection;
these functional checks do not certify arbitrary host runtimes.

Packaged games create and reuse `Cache/Shaders/<API>/` under the game root
(1=OpenGL, 2=Vulkan, 3=D3D12); configuration remains external at
`Config/Config.ini`. Cache identity includes source text, backend target and
recursively referenced literal HLSL includes. Editing an included file invalidates
the dependent shader. Macro-generated include dependencies are not fingerprinted.

Shaders use `.gasset` files with `type: shader`, `format: shader` and a text
payload. That payload is the `.shader` container: a `Pass` contains fixed render
state, stage pragmas and HLSL-like program code. Literal includes reference
`.gasset` files whose type is `shader-include`.

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
