# Shader authoring and reflection

Pipeline candidate failure follow-up (2026-11-01): failed DXC status, empty
output or missing stage/source now causes a runtime exception before reflection,
cache publication or native shader construction, including Release. GL/Vulkan
require successful SPIR-V and DXIL reflection compilation; DXIL success cannot
mask SPIR-V failure. DXC operation results/diagnostic/output blobs use owned
references released on all paths. Warning diagnostics with successful status
remain warnings. A well-formed shader container with invalid HLSL exercises
candidate rollback in Example/RenderPipeline. The container rewrite below adds
malformed-source validation separately; native draw-time pipeline errors remain
outside that scope. See [P1](../../TODO/Rendering/RenderPipeline.md) and the
[parser work item](../../TODO/Rendering/ShaderGraph.md).

## Container parser contract

The container front end is rewritten as a scanner/parser, exposed through
ParseShaderContainer in GEngine/Graphics/ShaderContainer.h. It validates the
entire decoded text before committing parsed Shader state or touching disk cache/compiler
state. It returns owned values or throws std::invalid_argument with source name,
one-based line and byte column. Debug/Release execute the same validation. Diagnostics
for .gasset refer to payload lines. This parser does not compile/validate HLSL
types, entry-point existence, preprocessor conditions or device capabilities.

- One nonempty quoted Shader name and one or more Pass blocks are required;
  each Pass ends with exactly one Program block. Unknown/trailing input is an
  error. Keywords and state/stage values are case-insensitive; entry names/Tags
  preserve spelling. Quoted values support escaped quote/backslash.
- Pass directives/pragmas occupy separate lines before Program. Whitespace,
  CRLF, leading UTF-8 BOM, line comments and block comments are supported. Quoted
  names cannot span lines; unterminated comments/quotes/blocks and NUL bytes fail.
  Program extraction preserves every byte inside its braces, including its last
  byte; braces inside comments and quoted/character literals do not end a block.
  Braces must balance lexically; conditional HLSL preprocessing remains DXC's job.
- Cull, DepthTest, DepthWrite, Blend, BlendOp, ColorMask and Tag may each appear
  once per Pass. Unknown commands/values, incorrect argument counts, duplicate
  directives/channels and malformed state values fail instead of defaulting or
  asserting. Tag accepts one bare or quoted nonempty value. ColorMask accepts
  individual R/G/B/A channels separated by spaces/commas, or 0 for none.
- DepthWrite accepts On/True/1 and Off/False/0; On now correctly enables writes
  (the old parser incorrectly disabled it). DepthTest keeps LEqual/GEQual aliases.
  Blend accepts two factors or four for separate color/alpha; BlendOp accepts
  one or two operations and keeps Substract/ReverseSubstract spellings while also
  accepting Subtract/ReverseSubtract. Omitted directives retain RenderState defaults.
- A stage is declared once with #pragma stage entry, using a valid identifier.
  Compute cannot mix with graphics. Amplification requires Mesh; Mesh cannot mix
  with Vertex/Hull/Domain/Geometry. Hull and Domain must pair. A graphics pass
  needs Vertex or Mesh; fragment output is optional for depth-only passes.

Cache hashes include parser revision 2, invalidating old compiled extraction
results without changing the disk format. Existing Shader::Create in-memory path/
name caching is not hot reload. Old #Name/#Type GLSL containers and shader fragments
incorrectly marked type: shader are explicitly rejected; this rewrite does not
add support for those already-incompatible assets.

## File format examples

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
