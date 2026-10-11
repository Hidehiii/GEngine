# Shader authoring and reflection

## File format

GEngine shader assets use a `.shader` wrapper around HLSL-like program source. A file contains one `Shader` block and one or more `Pass` blocks.

```hlsl
Shader "Example"
{
    Pass
    {
        Cull Off
        DepthTest LessEqual
        DepthWrite On
        Blend SrcAlpha OneMinusSrcAlpha
        BlendOp Add
        ColorMask R G B A
        Tag Forward

        #pragma vertex vert
        #pragma fragment frag

        Program
        {
            // HLSL declarations and entry points
        }
    }
}
```

The engine parses the state block, extracts the program body, compiles each declared stage for the active backend and stores a backend-specific cache entry.

## Pass state

| Directive | Values | Effect |
| --- | --- | --- |
| `Cull` | `Off`/`None`, `Back`, `Front` | Selects rasterizer face culling. Prefer `Off` for two-sided or diagnostic geometry. |
| `DepthTest` | `Less`, `Greater`, `LessEqual`, `GreaterEqual`, `Equal`, `NotEqual`, `Always` | Selects depth comparison. |
| `DepthWrite` | `On`, `Off` | Enables depth-buffer writes. |
| `Blend` | Source and destination factors | Enables color/alpha blend factors. |
| `BlendOp` | `Add`, `Substract`, `ReverseSubstract`, `Min`, `Max`, `None` | Selects blend operation. `None` disables blending. |
| `ColorMask` | Any of `R G B A` | Selects writable render-target channels. |
| `Tag` | A pass name | Metadata for higher-level material/render selection. |

State words are case-insensitive. Use the documented spellings for portable assets; the current parser accepts `Cull Off` and `Cull None` as aliases.

## Stage declarations

Use pragmas to select shader entry points:

```hlsl
#pragma vertex vert
#pragma fragment frag
```

Other supported stage names are `compute`, `hull`, `domain`, `geometry`, `amplification` and `mesh`, subject to backend and device support.

## Vertex input contract

The vertex shader input structure defines the vertex-buffer layout:

```hlsl
struct VsInput
{
    float3 position : POSITION;
    float2 uv       : TEXCOORD0;
};
```

The application must upload an interleaved buffer with matching order and size:

```cpp
struct Vertex
{
    float Position[3]; // POSITION: 12 bytes
    float Uv[2];       // TEXCOORD0: 8 bytes
};
```

The reflection system derives the byte offset and total stride from the vertex stage only. D3D12 maps `float`, `float2`, `float3`, `float4` to `R32_FLOAT`, `R32G32_FLOAT`, `R32G32B32_FLOAT`, and `R32G32B32A32_FLOAT` respectively.

Inputs named `TEXCOORD8` through `TEXCOORD15` are currently treated as per-instance data. All other vertex-stage inputs are per-vertex data.

## Resources and material properties

Constant buffers, textures, samplers and storage resources are reflected from compiled shader data. A `Material` owns the CPU-side property values and backend descriptors. Bind properties through the material API using the reflected name; do not assume descriptor-table offsets in application code.

For D3D12, reflection creates the root-signature descriptor ranges. Cached DXIL remains owned by `D3D12Shader`, so pipeline-state bytecode pointers remain valid after construction.

## Shader cache

The cache stores compiled stage code and reflection blobs under `ShaderCacheDirectory`. Cache validity is based on the shader source hash and active graphics backend.

When a shader source edit changes its hash, the engine recompiles it automatically. When changing engine reflection logic, loading an existing cache is sufficient because reflection is performed again from the stored reflection blob.
