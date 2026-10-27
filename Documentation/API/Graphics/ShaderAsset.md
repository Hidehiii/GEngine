## Shader asset

Packaged games create and reuse `Cache/Shaders/<API>/` under the game root
(1=OpenGL, 2=Vulkan, 3=D3D12); configuration remains external at
`Config/Config.ini`. Cache identity includes source text, backend target and
recursively referenced literal HLSL includes. Editing an included file invalidates
the dependent shader. Macro-generated include dependencies are not fingerprinted.

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
