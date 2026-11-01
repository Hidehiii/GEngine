## Backend layering and shader ownership

Shader container rewrite (2026-11-01): ParseShaderContainer is a CPU-only shared
front end in Graphics/ShaderContainer.h/.cpp. It returns owned name/pass state,
stage entry maps, mesh flags and exact Program bodies; it performs no graphics,
file/cache or compiler operations. A bounded scanner handles whitespace/BOM,
comments, quoted values and nested program braces without matching keywords in
comments/HLSL. Parse failures carry source/line/column in std::invalid_argument.
State/pragma duplicates, unknown values, invalid stage combinations and trailing
input fail before publication. Shader::Preprocess first builds temporary vectors,
then swaps all fields only on complete success. Parsing precedes disk-cache
access and folder creation, compilation, reflection and backend construction;
existing in-memory Shader::Create cache reuse is unchanged (no hot reload).
ShaderCacheSource includes parser revision 2 so old extraction results are not
reused after this change. Utils state converters keep their signatures but now
throw for unknown values. Documented DepthWrite On correctly enables writes.
GPU ownership, GL/Vulkan/D3D12 compilation and queue semantics are unchanged.
CPU Debug/Release/corpus checks pass; complete engine/example builds are blocked
by the separately required host D3D12 version guard. New pipeline rollback/cache
fixtures are implemented but their GPU/runtime acceptance remains unverified.

P3 in progress: RGBA16F maps to GL_RGBA16F, VK_FORMAT_R16G16B16A16_SFLOAT and
DXGI_FORMAT_R16G16B16A16_FLOAT. Format queries and graph/asset validation accept
the appended color enum without treating it as depth; existing enum values stay
unchanged. Imported texture schema/readback remains unchanged. Post stages use
ordinary sampled/attachment resources and existing submission retention; there
is no new native queue path. Debug/Release P3 numeric output and format-query/
metadata regressions passed separately on all three backends, with Vulkan shared
and dedicated queues. No native validation warnings/errors were captured; numeric
RGBA16F MSAA resolve and unsupported-device injection remain untested.

P2 foundation (2026-11-01, Debug/Release verified): command buffers accept a
compatible immutable render-pass override and retain its owner independently of
the framebuffer. This supports color/depth LOAD between cameras without replacing
the framebuffer's shared pass pointer while recording. Viewport/scissor is command
state with a portable top-left pixel origin. Vulkan pipelines consume that state
for each draw; GL translates Y and disables scissor for full clears/resolves and
presentation; D3D12 records native viewport/scissor commands. Existing full-target
defaults and submission retirement remain in place; no per-camera allocation or
device-wide idle is introduced during execution. Here allocation refers to GPU
resources; binding lookup uses ordinary CPU containers.
New Vulkan offscreen attachments now track UNDEFINED until their first actual
transition/render pass. Claiming attachment-optimal at allocation caused invalid
old-layout barriers for camera passes that explicitly begin their framebuffer.
Unrepresented RenderImage2DFormat conversion returns VK_FORMAT_UNDEFINED instead
of an undefined C++ return; explicit image aspects still drive layout barriers.
AttachmentUvStartsAtTop describes framebuffer sampling origin (false on GL,
true on Vulkan/D3D12), keeping consumer UV selection independent of native APIs.

Pipeline asset candidate preparation now propagates shader compilation failure
before reflection/cache publication/backend construction in both builds.
SPIR-V compilation and DXIL reflection must both succeed; DXC result/blob COM
references are scoped to the compilation operation and released on error too.
Native backend interfaces and runtime compiler ownership are unchanged; the
existing legacy shader-container parser is not comprehensively hardened by P1.
See [pipeline selection](RenderPipeline.md) for the failure rollback boundary.

Per-format/MSAA follow-up (2026-11-01) keeps native format queries,
multisample allocations, views and resolves backend-owned. Scope and acceptance
are in [Phase 4](../../../TODO/Rendering/Phase4.md#per-format-capabilities-and-msaa-2026-11-01).
Graph and device factories reject unsupported combinations before allocation;
no resource pooling or per-frame device wait is introduced.

Per-format implementation: OpenGL uses internalformat query2 support/role,
per-format sample-count queries and framebuffer size limits. Vulkan queries
optimal image usages for sampled resolved images and persistent multisample
attachments, intersects framebuffer sample masks and queries depth/stencil
resolve properties. D3D12 queries FORMAT_SUPPORT and MULTISAMPLE_QUALITY_LEVELS;
the native quality index is zero. Color resolve and depth resolve are separate.
No native handles or format enums escape into application code.

GL creates immutable single-sample attachments, correctly creates multisample
textures, resolves each MRT color separately and resolves applicable depth/
stencil aspects. Completeness checks execute in Release and construction
failure releases partial objects. D3D12 uses TEXTURE2DMS RTV/DSV views, resolves
colors and supported native depth reductions, keeping MSAA depth for later LOAD.
Vulkan creates each MSAA color with its own format, clears both resolved and
multisample depth slots and uses a depth-only sampled view for D24S8. D32
resolve uses no stencil resolve mode. New attachment view failures release
their image allocation, while tracked children use existing retirement.

Framebuffer attachment accessors return aliasing shared references that retain
the framebuffer owner. This keeps native allocations/views/descriptors alive
when later copy/compute/material submissions retain a texture after graph
Reset; the framebuffer holds the wrapper objects without an ownership cycle.
Depth-resolve/LOAD follow-up (2026-11-01): Vulkan retains
device-local MSAA backing with STORE, attachment-optimal LOAD layouts and explicit
attachment read/write dependencies. D3D12 uses typeless depth resources with typed
DSV/SRV views and graphics-list ResolveSubresourceRegion min/max reductions.
Native source returns to DEPTH_WRITE for later LOAD; resolved output follows
normal texture state tracking. Mode support is queried and validated per format;
render-pass caching and graph portable/legacy conversion include the selected mode.
D3D12 viewports keep [0,1] in both depth conventions and depth optimized clear
values match the active convention. Example coordinates account for OpenGL's
existing [-1,1] clip and reversed viewport mapping; no GL projection API changes.
Standard D3D12 sampler descriptors, including the material fallback, use NEVER
for the ignored comparison field, avoiding native comparison-filter warnings.
Ownership remains framebuffer-scoped and follows existing queue retirement;
acceptance and verification are tracked in Phase 4.

Partial D3D12 construction frees only valid descriptor allocations. Native
MSAA backing images remain framebuffer-owned, and device children still retire
after the existing queue completion tokens. No per-frame idle wait was added.
Vulkan preserved multisample LOAD and supported D3D12 depth reduction now use
the existing framebuffer lifetime; swap-chain MSAA uses the offscreen path.

MSAA MRT readback also exposed D3D12 arrays allocating a full native mip chain
while engine metadata and subresource calculations assumed one mip. Native array
creation now explicitly requests one mip, keeping layer indices consistent
for uploads, graph copies, barriers and readback without changing the API.
Wrapped float/integer/depth attachments not represented in RenderImage2DFormat
now report NONE consistently: D3D12 no longer aliases them to RGBA8 and Vulkan's
conversion has an explicit default. This prevents misleading copy validation;
native attachment formats remain independently available for rendering.

CMake Debug/Release engine/example builds and GL/Vulkan shared/D3D12/Vulkan
dedicated 120-frame MSAA runs passed, with 24 cases repeated three times each.
Debug 6000-frame runs also passed. Final runs captured no D3D12/Vulkan validation
diagnostics; Debug Vulkan had no unavailable-layer fallback. These are bounded
color resolve/depth test/retention checks, not numeric resolved-depth sampling
or indefinite memory proof. Existing D3D12 layer-on handle growth stays open;
validation was kept enabled rather than bypassed.

Depth/LOAD follow-up Debug/Release GL, Vulkan shared, D3D12 layer-on and Vulkan
dedicated runs passed separately at 120 frames. Three matrices per run total
1536 color cases, 1184 numeric depth probes and 432 mixed-coverage min/max edge
checks, plus actual-frame retained color/depth and recreation. Logs are
build/msaa-depth/verified-{Debug,Release}; no native warnings/errors were captured.
Dedicated Vulkan selected graphics=0, compute=2, transfer=5. Tier0 D3D12 hardware,
numeric stencil outputs, >8x rendering and injected allocation failure remain
outside this bounded verification; lifetime growth remains a separate open item.
Debug 6000-frame runs passed all four variants, with normal shutdown and clean
native diagnostics. Vulkan shared/dedicated handles were 791->793 / 836->837;
D3D12 layer-on remained growing (1577->41170), as recorded in the open diagnosis.

Foundation capability/lifetime follow-up is tracked in
[Phase 4](../../../TODO/Rendering/Phase4.md): color attachment limits now use
queried OpenGL/Vulkan values or D3D12's native required count, and D3D12 uniform
capacity converts 16-byte elements to bytes. The same work compares current-source
debug-layer lifetime controls. Backend allocation and queue ownership remain
unchanged by reporting/preflight work; allocation attribution remains open.

Current-source matched-frame D3D12 controls completed 30,000 frames with the
layer off/on and replacement off/on. All passed readbacks and normal shutdown.
Growth reproduced with validation enabled even without replacement; sampled
pending submissions were bounded (at most 15), deferred releases were zero,
and replacement traces showed 3 live pipelines after 1000 creations. Context
reports still include retained objects and do not establish a leak or its
owner. See [the open diagnosis](../../../TODO/Integration/D3D12Memory.md).

Packaged shader cache files live in game-root Cache/Shaders/<API>/ and persist
between launches. Backend directories prevent switching APIs from clearing another
backend's cache. Cache fingerprints include recursively referenced literal HLSL
include contents as well as source and target settings. This CPU file-cache change
does not alter shader module ownership, GPU synchronization or public factories.

Application and renderer code use `GEngine/include/GEngine` interfaces.
`GEngine/src/Platform` and `GEngine/include/Platform` contain OpenGL, Vulkan,
and D3D12 resource, command, context, and presentation implementations;
`Surface` contains GLFW and Win32 windows/input adapters. Native handles,
allocation policies, and synchronization remain behind those boundaries.
Vulkan engine-owned buffers, images, staging uploads, and readbacks use the
context-owned VMA allocator from the official Vulkan SDK. Allocation handles
are backend-private; resource retirement destroys them after submitted GPU
work and before the allocator/device.

OpenGL backend construction selects an API without issuing GL calls. Window
creation makes the context current and loads GLAD; `GraphicsRuntime::Initialize`
then sets up debug output/default GL state. Loader failure throws before a GL
state call, including in Release. OpenGL translates the compiler's Vulkan
SPIR-V intermediate to desktop GLSL 450 using SPIRV-Cross; it does not submit
Vulkan binaries to `glShaderBinary`. Shader compile/link failures throw and
release partial programs. Both vertex-buffer constructors create a VAO and
destruction deletes it; non-indexed draws do not require an index buffer.
Separate Texture2D/cube-map and sampler reflection is translated into combined
image samplers; material code binds both resources to the image unit. Other
complex texture/sampler combinations remain explicitly unsupported until they
gain focused coverage.

`Shader` owns reflection data and compiled bytes. Reflected vertex inputs set
buffer format/offset/stride; D3D12 uses DXIL reflection and Vulkan uses SPIR-V
reflection. `Shader::Create(path)` checks a cache keyed by active API and
lexically normalized absolute path before backend construction. Relative
paths resolve against the working directory; symlinks/case variants are not
canonicalized. `GetShader(name)` is a compatibility lookup, and
`ShutdownCache` clears both caches while the context is alive. Cache lookup
is render-thread-only, runtime-scoped, and not hot reload.
