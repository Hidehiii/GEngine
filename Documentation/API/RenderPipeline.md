# Render pipeline API

P3 is implemented and programmatically verified (2026-11-01). PostProcessStackPass
consumes a named linear RGBA16F composed target and writes a distinct RGBA8
output through ordered exposure/Reinhard stages. Payload v3 carries effect IDs,
enabled flags, EV and explicit Linear/sRGB output encoding. A live settings
provider may change parameters/order without changing stage count; P1 replacement
handles resize/topology changes. See the P3 TODO for acceptance and limitations.

Include `GEngine/Renderer/PostProcess.h`. RegisterPostProcessPass(registry,
optionalProvider) registers stable pass ID `GEngine.PostProcess.v1`; its sole
setting is `{stack: Name}`. RenderPipelineAsset::PostProcessStacks and
GetPostProcessStack(name) hold/resolve v3 declarations. Each declaration names
Input, Output and a typed shader resource key plus PostProcessSettings. Settings
contain Enabled, Encoding (Linear/SRGB) and up to four ordered effects, each
with a unique ID, Exposure/Reinhard Type, Enabled and EV. Empty effects still
record one copy. Input must be one single-sample linear RGBA16F attachment;
output must be distinct, same-size single-sample RGBA8 without depth. Unsupported
format/device combinations and incompatible shader contracts throw explicitly.

Input is finite straight linear RGB; premultiplied-alpha correction is not provided.
Exposure applies min(max(rgb,0)*exp2(EV),65504), EV finite in [-16,16]. The clamp
keeps the half-float intermediate finite. Reinhard applies x/(1+x) to nonnegative
RGB; alpha is preserved. A disabled stage/global bypass copies nonnegative RGB.
Final linear output clamps through RGBA8 UNORM; SRGB additionally applies the
standard 0.0031308 piecewise transfer once at the final stage. Output is stored
in UNORM, not an sRGB attachment; consumers must not encode it a second time.
This describes stored bytes, not OS/display color management.

The shader dependency implements the v1 contract demonstrated in
Example/RenderPipeline/Assets/Shaders/PostProcess.gasset: one graphics pass,
float3 POSITION/float2 TEXCOORD0 vertices, PostInput texture2D, PostSampler and
float4 PostParameters (operation 0=copy/1=exposure/2=Reinhard, exposure multiplier,
final-sRGB flag, reserved). Prepare validates reflected material properties and
sets depth/cull/blend state; the application supplies/deploys the shader resource
and is responsible for retaining its specified math. Sampling is nearest/clamp;
filtered HDR format support is not assumed.

The optional provider returns a settings snapshot during Execute before graph
recording. Count and ID/type set must match the configured stack, while order,
enabled flags, EV and Encoding may change. Invalid snapshots fail the pipeline
before recording; Reset/Clear is required, as with P2 binding failure. Every slot
has separate material parameter storage. Topology/count/resize changes require
P1 replacement. N slots own max(N-1,0) HDR intermediate targets, including when
disabled; no per-frame GPU targets, pooling, aliasing, history or compute path.
Command capacity must cover the full application graph plus diagnostic graphs.
GetIntermediateTargets returns graph handles after Configure; resources resolve
after Prepare and remain graph-owned, not owned by the returned handle vector.
Reconfiguring the same pass for an equivalent immutable asset layout clears old
stage/material/target references before rebuilding. It does not permit concurrent
use by multiple active graphs, or reuse with a different asset context/layout.

P3 Debug/Release engine/example builds and separate 120-frame OpenGL, Vulkan
shared, D3D12 and Vulkan dedicated runs passed 18 invalid candidates, four invalid
live snapshots, two live markers, six replacements and 11 reference probes each.
Debug 6000-frame runs passed 349 replacements/403 probes per variant, with exact
graph-owned target counts/identities and clean native diagnostics. Both build
matrices also passed repeated preparation with the same pass/equivalent layout.
P1/P2, original
C++ and MSAA regressions passed both builds on all four variants. The P3 TODO
records paths, bounded memory observations and untested cases (including manual
window/display encoding, three/four-slot chains, HDR MSAA and packaged runtime).

P2 camera stacks are implemented and programmatically verified (2026-11-01), tracked in the pipeline
TODO. The v2 payload adds named base/overlay trees and stable application
camera IDs; v1/P1 assets remain supported. Viewports use normalized top-left
coordinates; clear operates on whole attachments and overlays preserve through
LOAD/STORE. Explicit per-camera graph work feeds one composed target. Runtime
camera bindings remain caller-owned; no automatic scene discovery or P3 effects.

Include `GEngine/Renderer/CameraStack.h`. CameraStackSpecification contains Name,
Target, BaseCamera and up to 64 Nodes. Nodes carry Camera ID, Base/Overlay role,
Enabled, ordered Overlays edges, viewport and Color/Depth begin operations;
Target/Samples can be inherited or must exactly match the stack target. Base
clears existing attachments. Overlays clear or load color/depth; no-depth targets
require Depth=None. STORE is fixed. Trees reject cycles, duplicate/shared cameras,
disconnected nodes, missing children, extra bases and invalid/empty viewports.
Actual graph/command capacity may be stricter than the 64-node schema bound.

RenderPipelineAsset Version=2 serializes CameraStacks; v1 continues to load and
serialize unchanged and rejects stack fields. GetCameraStack(name) resolves named
declarations. Registered pass factories can construct CameraStackPass(name,stack,
context,bindings,recorder). Configure declares per-camera graphics writes and
serial edges; Prepare creates compatible immutable render passes; Execute resolves
bindings before any graph recording. The final named target is the composed output
for a single presentation/effects consumer. Multiple-target composition is rejected.

The example checks both raw composition and an offscreen sampled copy, with
fullscreen UVs selected from GraphicsCapabilities::AttachmentUvStartsAtTop.
The sampled copy also checks that later draws reset the overlay viewport.

Bindings return stable IDs, borrowed CameraComponent pointers and Active flags.
Pointers must remain valid through pipeline Execute; they are not retained by GPU
commands. Missing/null active cameras or duplicate bindings throw before submission.
Disabled/inactive parents suppress descendants; missing suppressed bindings are
allowed. An inactive base clears output once and records no cameras. Explicit
binding Active/Enabled controls activity; isPrimary, RenderOrder and vector order
do not implicitly create stacks. Recorder callbacks own scene draw/projection logic
and must use independent per-camera GPU parameter storage. Runtime binding failure
marks the pipeline failed and requires Reset/Clear; it is not failed-activation
rollback. Viewport edges round to pixels, clears affect whole attachments, and
later cameras preserve data only through LOAD/STORE on the same target.

Debug/Release GE_WITH_MONO=OFF builds and separate 120-frame GL, Vulkan shared,
D3D12 and Vulkan dedicated runs passed. Each checked 16 invalid candidates,
three invalid bindings, two live activity changes without replacement, seven
replacements and 11 paired raw/sampled pixel probes. Debug also passed 6000
frames per variant (399 replacements/403 probes). P1, original C++ pipeline and
MSAA regressions passed in both builds on all four variants, with clean native
diagnostics. See the P2 TODO for logs, bounded memory samples and untested manual,
archive/export, Mono/Premake integration and device-failure scenarios.

P1 asset/switching is implemented (2026-11-01), tracked in
[the pipeline TODO](../../TODO/Rendering/RenderPipeline.md). Additions
include a versioned GAsset payload, runtime-owned pass factories, candidate Prepare
and frame-boundary transactional selection. Camera ownership remains with the
caller; P2/P3 are not part of the P1 milestone. Existing APIs below describe
the lifecycle baseline and the verified P1 contract.

Include `GEngine/Renderer/RenderPipelineAsset.h` for the asset API.
RenderPipelineAsset loads/saves the existing GAsset envelope (`render-pipeline`
type/format) and parses/serializes a version-1 YAML payload. It contains fixed-size
named targets, typed runtime-root-relative GAsset dependencies, scalar settings,
and ordered named pass type IDs. Only `GEngine.Composable` is supported in v1.
See the concrete [warm asset](../../Example/RenderPipeline/Assets/Pipelines/Warm.gasset)
and [cool asset](../../Example/RenderPipeline/Assets/Pipelines/Cool.gasset).
Target load/store is clear/store and depth resolve uses BackendDefault; stacking
and arbitrary subpasses are not represented. Unknown/duplicate fields, invalid
IDs, formats, dimensions, sample counts and path escapes are rejected.

`RenderPipelineAssetRegistry::Register(type, validator, factory)` registers
runtime-owned pass definitions. Create takes an asset snapshot and checks all
dependencies, target capabilities and validators before invoking factories.
Validators must enforce their pass-specific settings, dependencies/order and
capabilities. Factories receive an immutable context with named target handles;
returned names must match the asset. No native API handles are serialized.
Registrations are copied before validators run, so callback-driven registry
replacement does not invalidate candidate creation. Registries are not global
and can be destroyed after the captured candidate is created.
The example alternates submission and reset of recorded outgoing-target work;
OpenGL issues commands immediately, so resetting its wrapper is not GPU rollback.
Create performs preflight and returns an unprepared instance; call Prepare
before direct use, or use the selector to prepare and activate transactionally.

`RenderPipeline::Prepare()` configures the pass list, compiles the graph, then
calls each pass's optional `Prepare(const RenderGraph&)` exactly once to build
resources that require allocated targets. Execute prepares automatically. The
hook cannot declare graph work; all declarations belong in Configure. Owning
pipelines are noncopyable; asset data remains copyable. Preparation
failure requires Reset and does not submit graph work. Shader/native draw-time
errors remain runtime errors; Prepare does not promise successful GPU execution.
Well-formed shader containers with invalid HLSL now raise an exception before
reflection/cache/native construction, preserving selection in Debug and Release.
Comprehensive legacy shader-container directive/parser hardening is separate.

`RenderPipelineSelector::Activate(assetOrPath,registry)` prepares an isolated
candidate before replacing the current instance. A load/validation/factory/
configuration/graph/preparation exception preserves the active instance/settings.
Activate/Clear are rejected inside hooks or between Execute/Present. The selector
is noncopyable, owns no cameras, and forwards the caller's camera vector unchanged.
GetActive returns a const owning instance reference for inspection. Settings are
immutable snapshots; resizing replaces a candidate with changed target dimensions.
Execute/Present failures mark the selector failed; discard partially recorded
caller commands, then Clear to recover. Registry/loading/selection are render-
thread operations with the active graphics runtime, not concurrent async APIs.
Rebuild engine, applications and pass plugins together: Prepare adds a virtual
pass hook, and owning pipeline objects are now noncopyable.

P1 CMake GE_WITH_MONO=OFF Debug/Release four-backend 120-frame runs passed 18
failed candidates, seven switches and nine numeric output probes per run, with
registry snapshot, camera/frame guards and submitted/discarded outgoing work.
Debug also passed 6000 frames per variant (399 replacements/401 pixel probes).
Original lifecycle/compute mode passed again in both builds. Native diagnostics
were clean; the fixture intentionally logs one HLSL compiler failure per run.
See the TODO for logs, memory observations and untested build/archive/UI variants.

Lifecycle follow-up (2026-10-31): implementation and verification status are
tracked in [the pipeline TODO](../../TODO/Rendering/RenderPipeline.md).

[API index](README.md) · [Render pipeline architecture](../Architecture/Rendering/RenderPipeline.md)

Include `GEngine/Renderer/RenderPipeline.h`. The abandoned
`GEngine/Graphics/RenderPipeline.h` two-method interface is removed.

## Core types

```cpp
class RenderPipelinePass
{
public:
    virtual const std::string& GetName() const = 0;
    virtual void Configure(RenderPipelineBuilder& builder) = 0;
    virtual void Execute(RenderPipelineContext& context) = 0;
    virtual void Present(RenderPipelineContext& context);
};
```

`RenderPipeline` owns a `RenderGraph` and ordered pass list:

```cpp
RenderPipeline pipeline;
pipeline.AddPass(pass);
pipeline.Execute(GraphicsPresent::GetCommandBuffer(), cameras);
pipeline.Present(GraphicsPresent::GetCommandBuffer(), cameras);
```

Derived pipelines normally override `Configure(RenderPipelineBuilder&)` to
create targets/resources and add passes. The framework then calls every
registered pass's `Configure` exactly once in insertion order, including passes
added externally before first execution. Do not configure passes manually in
the derived hook. If a pass consumes another pass's output version, resolve it
in the consuming pass's `Configure`, after the producer has configured.
`RenderPipelineBuilder` wraps graph
target creation and graphics/compute/transfer pass creation.
`RenderPipelineContext` exposes the graph, current command buffer, cameras, and
zero-based frame index. Each successful `Execute` must be followed by exactly
one `Present`; both receive the same frame index. The index advances only after
successful presentation. Present-before-execute, double execute/present and
reentrant phase calls throw `std::logic_error`, including Release builds.

`AddPass` and `RemovePass` are valid only before configuration or inside the
pipeline's configuration hook. Once configured, call `Reset()` between complete
frames before changing passes or target dimensions. Reset clears graph, passes
and frame index; the next Execute invokes the derived Configure again. External
passes must be re-added. Reset inside hooks or between Execute and Present is
rejected. A hook/graph exception marks the pipeline failed; subsequent frame
calls are rejected until Reset. Callers must abandon any partially recorded
presentation command when handling a presentation failure. Null command buffers
and invalid pass registrations are rejected independently of assertions.

The graph and pass callbacks own CPU resource references. Submitted command
resources remain retained by the backends until completion; Reset is not a GPU
wait. Direct graph access must respect the same between-frame rebuild boundary.

The example in `Example/RenderPipeline` demonstrates a complete compute,
offscreen graphics, and sampled presentation pipeline. It verifies the
storage-buffer result at frames 29, 59 and 119, rebuilds offscreen dimensions
at frames 30 and 60, and exits normally at frame 120. This is a
first composable-pipeline milestone. The opt-in P1 mode adds asset selection;
the opt-in P2 mode adds explicit camera stacking, and P3 adds ordered HDR effects.
Editor authoring remains unimplemented.

The lifecycle/rebuild checks passed in isolated GE_WITH_MONO=OFF CMake Debug
and Release builds on OpenGL, Vulkan shared/dedicated queues and D3D12 for
120 frames each; Debug also passed 6,000 frames per variant. These results
cover the frame/configuration contract and numeric compute result, not final
presentation pixels, actual window interactions or script-enabled integration.

## Planned extensions

[Pipeline TODO P1-P3](../../TODO/Rendering/RenderPipeline.md#planned-pipeline-extensions-2026-10-31)
tracks pipeline assets/runtime switching, explicit base/overlay camera stacks,
and an ordered post-processing stack starting with exposure and tone mapping.
The bounded P1/P2/P3 implementations are described above.
Pass factories still use C++; only explicit P2 declarations/bindings create
stacks. Plain sampled presentation does not provide effect processing; explicit
P3 declarations select the ordered effect pass and intermediate formats.
