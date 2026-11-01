## Device capabilities

P3 adds FrameBufferTextureFormat::FRAME_BUFFER_TEXTURE_FORMAT_RGBA16F (appended
without renumbering existing values), with queried attachment/sample/resolve and
unfiltered sampling support on GL/Vulkan/D3D12. It is an attachment format, not
a new imported-texture or CPU readback format. P3 Debug/Release four-backend
numeric runs and MSAA format-query/1x metadata regressions passed; post-processing
uses nearest sampling and single-sample HDR targets. Numeric RGBA16F MSAA resolve
remains untested; querying native support does not constitute that runtime check.

`GraphicsCapabilities::AttachmentUvStartsAtTop` describes native framebuffer
attachment sampling: true on Vulkan/D3D12 and false on OpenGL. A fullscreen
consumer can choose top/bottom vertex UVs from this value without inspecting the
backend identity. It does not describe imported asset texture orientation or
change texture-array readback row order. Rebuild clients after this capability
snapshot layout change.

The depth-resolve/LOAD follow-up in Phase 4 is implemented and verified below.
RenderPassSpecification and graph AttachmentSpecification accept DepthResolve
(DepthResolveMode). Call QueryRenderTargetFormat(format).SupportsDepthResolve(mode)
before using MSAA depth. DepthResolveModes is the bit mask of enum values;
DepthResolve reports BackendDefault support. Select one mode, not a bitwise
combination, in a target specification. Modes are irrelevant without MSAA depth.
BackendDefault preserves GL's implementation-selected depth sample and Vulkan's
sample-zero behavior; D3D12 selects Minimum for conventional depth and Maximum
for reversed depth. Minimum/Maximum mean numeric sample reduction and SampleZero
means sample index zero. OpenGL only supports BackendDefault; Vulkan queries
native depth/stencil mode combinations; D3D12 queries OPTIONS2 tier >= 1 for
native min/max. Unsupported modes fail before allocation. D24S8 stencil uses
MIN on D3D12 and sample zero on Vulkan, or the same mode when independent
depth/stencil resolution is unavailable. Stencil sampling is not exposed.
Vulkan now reports preserved MSAA LOAD with STORE; a first LOAD without earlier
stored contents remains undefined. D3D12 depth outputs have sampled typeless
backing and resolve on the graphics queue, without a compute shader or CPU wait.
D3D12 uses a [0,1] viewport range for both conventions: shaders/projections supply
the desired depth, while the reverse setting selects clears and default resolve.
Depth sampling uses ordinary texture/sampler bindings. Standard D3D12 samplers
and the material fallback now initialize their ignored comparison field without
native warnings; no new comparison-sampler API or filtering promise is added.

Per-format/MSAA follow-up (2026-11-01) is tracked in
[Phase 4](../../../TODO/Rendering/Phase4.md#per-format-capabilities-and-msaa-2026-11-01).
It implements device-owned attachment format queries and explicit resolve
support; signatures, validation and bounded verification are recorded below.

`RenderDevice::QueryRenderTargetFormat(FrameBufferTextureFormat) const` returns
`RenderTargetFormatCapabilities` for each existing attachment format. Invalid
or NONE formats return an empty result. ColorAttachment/DepthStencilAttachment
identify native attachment roles, Sampled describes unfiltered texture access,
and SampleCounts is a mask of power-of-two counts (1 | 2 | 4 | ... | 64).
Use `SupportsSamples(count)` together with ColorResolve for multisampled colors.
MaxWidth/MaxHeight include format/image and framebuffer limits. Sampled does
not promise linear filtering, a storage-image usage, or CPU copy/readback for
every format. This query covers attachment formats, not new texture schemas.

```cpp
auto& device = Graphics::GetRenderDevice();
const auto support = device.QueryRenderTargetFormat(FRAME_BUFFER_TEXTURE_FORMAT_RGBA8);
if (support.SupportsSamples(4) && support.ColorResolve) {
    RenderGraph::AttachmentSpecification target{};
    target.ColorFormats = { FRAME_BUFFER_TEXTURE_FORMAT_RGBA8 };
    target.Samples = 4;
    // Declare the target through the graph; all attachments must support 4x.
}
```

`ValidateRenderTarget(spec,width,height)` checks every attachment and dimension
before allocation. Graph Compile and direct device render-pass/framebuffer/
resize factories use it. Unsupported formats/counts/resolves fail with
`std::invalid_argument` in Debug and Release; no sample count is downgraded.
Offscreen colors resolve to the single-sample Texture2D returned by
GetRenderTarget. Texture references retain the framebuffer owner through later
sampling/copies and submitted queue work. GetRenderTargetCount counts logical
colors, including for MSAA, rather than backing/resolved image pairs.

DepthResolve and MultisampleLoad are separate capabilities. Supported D3D12
MSAA depth now resolves with native graphics commands; Vulkan D24S8 uses a
depth-only sampled view. All three backends preserve stored MSAA attachments
for LOAD. Clear/discard MSAA rendering remains supported. Swap-chain
MSAA is not exposed: render offscreen, resolve, then sample into presentation.

Native references: [OpenGL internal format queries](https://registry.khronos.org/OpenGL/extensions/ARB/ARB_internalformat_query2.txt),
[D3D12 depth reduction](https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12graphicscommandlist1-resolvesubresourceregion),
[D3D12 resolve feature gate](https://github.com/microsoft/DirectX-Specs/blob/master/d3d/ProgrammableSamplePositions.md#hardware-tiers),
[Vulkan resolve attachments](https://docs.vulkan.org/spec/latest/chapters/renderpass.html).

Query after graphics initialization on the render thread, with the active GL
context current where applicable. Rebuild engine and applications together for
the new RenderDevice virtual interface and framebuffer ownership base.
CMake GE_WITH_MONO=OFF Debug/Release runs passed GL, Vulkan shared/dedicated and
D3D12 120-frame matrices, three 24-case MSAA matrices per run; Debug also passed
6000 frames per variant. RGBA8 masks were GL=63 and Vulkan/D3D12=15. Counts
1/2/4/8 were rendered with MRT/depth and pixel checks; higher queried counts,
numeric resolved-depth values, script integration and Premake were not verified.
These are the initial color-MSAA results. The depth/LOAD follow-up adds
Debug/Release GL, Vulkan shared/dedicated and D3D12 120-frame checks, totaling
1536 MRT color cases, 1184 numeric depth probes and 432 min/max edge reductions.
Every run also retains color/depth across actual frames 1/59/61/119 with target
recreation at 60. Native diagnostics are clean; OpenGL uses its existing nearest
texture/sampler path. Counts above 8, Tier0 D3D12 devices, numeric stencil values,
script integration and Premake remain unverified. See Phase 4 for final stress
and existing RenderPipeline evidence. Debug 6000-frame runs passed every backend
variant; Vulkan steady handles stayed near-flat. D3D12 layer-on handle growth
remains separately unresolved and is not hidden by these correctness checks.

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

`GraphicsCapabilities::MaxColorAttachments` reports the usable color attachment
count: OpenGL takes the minimum of queried GL_MAX_COLOR_ATTACHMENTS and
GL_MAX_DRAW_BUFFERS, Vulkan uses maxColorAttachments, and D3D12 uses its required
SIMULTANEOUS_RENDER_TARGET_COUNT. QuerySources records the source and value.
`Graphics::GetMaxUniformBufferSize()` returns bytes on all backends; D3D12
multiplies its 4096 constant-buffer elements by 16, reporting 65536 bytes.
Existing factory and queue signatures are unchanged. Verification and remaining
format/sample-specific capability gaps are tracked in
[Phase 4](../../../TODO/Rendering/Phase4.md).

Current-source CMake Debug/Release FrameGraphTriangle runs on OpenGL, Vulkan
shared/dedicated and D3D12 reported attachment count 8 and uniform capacity
65536 bytes, passed target preflight and all three GPU readbacks, and exited
normally. This verifies reporting/preflight for this device, not every format,
sample count or resource variant. Lifetime diagnostics add no public signatures;
D3D12 validation-mode allocation ownership remains unresolved.
