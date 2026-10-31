# Shader authoring and reflection

## File format

GEngine shader assets use the `.gasset` YAML envelope with `type: shader`,
`format: shader` and a text payload. The payload is the `.shader` authoring
container around HLSL-like program source and contains one `Shader` block with
one or more `Pass` blocks. Raw `.shader` files remain a compatibility input for
advanced hand-authored projects, but first-party examples use `.gasset`.

```yaml
schemaVersion: 1
type: shader
format: shader
payloadEncoding: text
payload: |
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

The unescaped payload is the legacy `.shader` structure:

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

Shader includes must reference `.gasset` files with `type: shader-include`.
The include handler unwraps the YAML payload before DXC consumes it.

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
Separate `Texture2D` and `sampler` declarations remain portable material
properties. D3D12 and Vulkan bind them as distinct descriptors; OpenGL builds
combined image samplers with SPIRV-Cross and maps the sampler to the image's
texture unit. Applications continue using the reflected HLSL names.

## Shader cache

The cache stores compiled stage code and reflection blobs under `ShaderCacheDirectory`. Cache validity is based on the unescaped shader source hash, recursively included `shader-include` payload text and active graphics backend.

When a shader source edit changes its hash, the engine recompiles it automatically. When changing engine reflection logic, loading an existing cache is sufficient because reflection is performed again from the stored reflection blob.
