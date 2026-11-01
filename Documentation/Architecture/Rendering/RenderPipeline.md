# Render pipeline framework

P3 is implemented and programmatically verified (2026-11-01). The flow is composed HDR camera
output -> separate graph-owned RGBA16F intermediates -> RGBA8 final output ->
presentation. Fixed stage topology allows live settings without new GPU targets;
each stage declares sampled input and attachment output. Exposure/Reinhard operate
in linear RGB, with one explicit final encoding. No compute, aliasing or pooling
is introduced. Asset v3 and three-backend RGBA16F format support belong to this work.

PostProcessStackPass creates a fixed graphics stage per effect (one copy if
empty), declaring the previous color attachment as FragmentShader/ShaderRead
and a distinct attachment output. Intermediate RGBA16F targets are graph-owned
and sized from the composed input. The last stage writes the named RGBA8 output;
presentation reads it after the chain. Graph ordering and native framebuffer
transitions provide memory dependencies, including identical successive usage
states; no in-place read/write resource is permitted. Stage materials and a
shared quad/sampler are prepared before activation and follow existing backend
command retention across replacement and shutdown. Each material owns its own
parameter buffer so effects do not overwrite one another while recording.

The v1 shader contract is application-deployed and reflected during preparation;
the shipped example supplies exposure/Reinhard and final encoding math. Exposure
clamps to finite half-float range. Live settings are validated as one snapshot
before any stage parameter update; identities/types and count remain fixed while
order and parameters may vary. Disabled slots copy and retain their allocations;
empty stacks copy once. Pipeline replacement changes topology/dimensions. This
milestone does not add automatic scene discovery, effect plugins, editor UI,
temporal state, HDR swapchains or display color management.
Configure discards previous source handles, intermediates, materials and pipelines
before rebuilding an equivalent graph. Reusing a pass never shares live stage
state across active graphs; factories must create distinct passes for concurrent
candidates or changed asset contexts. A repeated-preparation regression checks
that old intermediate declarations and materials cannot accumulate.

Debug/Release builds and separate 120-frame P3 runs passed on OpenGL, Vulkan
shared, D3D12 and Vulkan dedicated. Each checked 18 rejected candidates, four
invalid live settings before recording, two same-instance markers, six replacements
and 11 reference probes. Debug 6000-frame runs passed 349 replacements/403 probes
per variant, checking fixed graph-owned target identities/counts each frame.
Native diagnostics were clean, including the final pass reconfiguration regression.
P1/P2, original C++ and MSAA regressions passed
all four variants in both builds. The P3 TODO records log paths, bounded memory
samples and untested manual/display, packaged, extended-chain and HDR MSAA cases.

P2 implementation and programmatic verification complete (2026-11-01): versioned stack declarations
validate base/overlay topology, target compatibility and viewports before P1
activation. Camera IDs resolve to caller-owned bindings before graph submission.
Each camera records explicit graphics work against the shared stack target with
compatible immutable render-pass operations and viewport/scissor. Commands retain
pass and framebuffer owners; no shared framebuffer operation mutation, GPU wait
or post-processing is added. Scope and acceptance are recorded in P2.

Stack nodes form a validated ordered tree (64-node schema ceiling) against one
logical target. v2 payloads carry stack metadata; v1 parsing/serialization remains
unchanged. Pass factories resolve a named stack from the immutable P1 context.
The reusable CameraStackPass declares one graphics write per node, serial edges,
and shared target ownership. Depth remains attachment-owned and ordered by these
edges/native pass transitions; no pooling/aliasing or independent depth scheduling
is enabled. Disabled passes remain conservative graph writes. Inactive bases clear
the target deterministically and suppress all callbacks. Providers resolve borrowed
camera identities before graph submission; callbacks consume them synchronously,
while native material/pipeline/target references survive GPU work separately.

The example samples the composed target into a separate diagnostic target before
presentation, checking both targets numerically. Native attachment UV origin is
described by AttachmentUvStartsAtTop (GL bottom, Vulkan/D3D12 top); consumers
select vertex UVs from the capability. This sampling copy is not a P3 effect.

Final Debug/Release engine/example builds and 120-frame runs passed separately
on GL, Vulkan shared, D3D12 and Vulkan dedicated, including 16 invalid candidates,
three invalid bindings, two same-instance activity changes, seven replacements
and 11 paired-output probes each. Debug 6000-frame runs passed 399 replacements
and 403 probes each; bounded memory samples are in the P2 TODO. P1, original
pipeline and MSAA regressions passed all four variants in both builds with clean
native diagnostics. Manual final-window pixels, Mono/Premake runtime, archive
dependency discovery, OOM/device loss and indefinite stability remain unverified.

P1 implementation (2026-11-01): an immutable asset/registry snapshot
configures and compiles a candidate, prepares its presentation resources, and
replaces selection only between complete frames. Failed candidate construction
leaves the active pipeline untouched. Submitted resources continue through
existing backend retention; no per-frame device idle or pipeline retirement list
is introduced. Cameras remain caller-owned. Scope and acceptance are in the P1 TODO.

GAsset bytes resolve through ResourcePack;
the versioned composable payload holds fixed targets, typed dependencies, scalar
settings and stable pass IDs. An explicit per-runtime registry validates every
pass before creating a candidate. Target handles live in an immutable context;
Configure declares work, graph compilation allocates targets, then pass Prepare
builds target-dependent presentation resources without new graph declarations.
The selector swaps a prepared candidate only at an idle frame boundary and
keeps the active instance on any candidate exception. Asset/settings and factory
snapshots outlive loading/registry mutations. Registrations are captured before
validators run, including callbacks that replace the source registry; factories
remain owned by the candidate through preparation. Camera references are forwarded,
not serialized, transferred or reset by selection. Native draw-time failures
remain runtime errors, with explicit failed-selector recovery.

No outgoing-pipeline list or device idle is added. Backend command retention and
retirement own GPU lifetimes after the old CPU graph/pass list is released.
Vulkan BeginRenderPass now retains its framebuffer, covering the previously
unsafe recorded-but-not-submitted replacement path as D3D12 already does.
The asset fixture alternates submitting and discarding recorded outgoing work;
native command owners release on safe reuse. OpenGL's immediate command path
keeps its existing semantics rather than emulating deferred cancellation.

P1 verification passed GEngine/RenderPipeline CMake GE_WITH_MONO=OFF builds and
120-frame asset runs on GL, Vulkan shared, D3D12 and Vulkan dedicated separately
in Debug/Release. Eight runs total 144 rejected candidates, 56 replacements and
72 numeric output probes, with registry mutation, camera/frame identity,
target resizing and recorded outgoing work. Debug 6000-frame runs added 399
replacements/401 probes per variant. Native diagnostics were clean; intentional
HLSL errors were recovered before activation. Original lifecycle/compute mode
passed again in both builds. Final evidence: build/pipeline-assets/final-
{Debug,Release} and final-stress-Debug. This does not verify indefinite memory,
all shader-container parser errors, Mono/Premake, archive/export or final-window
pixels; the separate D3D12 validation-growth diagnosis remains open.

Lifecycle follow-up (2026-10-31) centralizes pass configuration
and freezes the graph between explicit resets. Rebuild and dimension-change
verification is tracked in [the pipeline TODO](../../../TODO/Rendering/RenderPipeline.md).
Native submission/resource retention remains backend-owned; this work adds no
per-frame device waits or pooling.

[Rendering architecture](../Rendering.md) · [Render pipeline API](../../API/RenderPipeline.md)

`RenderPipeline` is the backend-neutral composable pipeline layer above
`RenderGraph`. The old two-method `Graphics/RenderPipeline` contract is removed.
The new public header is `GEngine/Renderer/RenderPipeline.h`.

A pipeline owns one persistent `RenderGraph`, an ordered list of
`RenderPipelinePass` objects, and an explicit configure/execute/present flow:

1. A derived pipeline overrides `Configure(RenderPipelineBuilder&)`.
2. It creates targets/resources and adds ordered pass objects.
3. The framework configures all registered passes in insertion order, once per
   graph generation. Each pass builds graph passes and declares resource versions.
4. `Execute(completion, cameras)` calls per-frame pass hooks and then executes
   the graph through `RenderGraph::ExecuteGpu`.
5. `Present(command, cameras)` calls pass presentation hooks. Presentation
   remains backend-neutral through `GraphicsPresent`.

Configuration, execution and presentation are guarded phases. A configured
pipeline rejects pass-list mutation; rebuilding explicitly clears the graph,
passes and frame index with Reset between complete frames. Derived Configure
must recreate generation-specific resources; external passes are re-added.
Execute/Present share one frame index, advanced after Present. Invalid and
reentrant calls fail in Debug and Release. A configuration/frame exception
leaves a failed state until explicit Reset, avoiding retries over a partial
graph. Presentation failure recovery also requires discarding the partially
recorded caller-owned presentation command.

Reset clears graph callbacks before pass owners and does not wait on the
device. Existing backend command/submission retention owns GPU lifetime for
submitted target textures, materials, pipelines and buffers. Rebuild is not
allowed from hooks or between Execute and Present. This is explicit offscreen
target rebuilding, not automatic swap-chain resize or camera stacking.

`RenderPipelineBuilder` wraps graph target/pass creation. `RenderPipelineContext`
carries the graph, current command buffer, camera list, and frame index. This is
a first composable framework, not full Unity URP/HDRP parity. P1 adds explicit
asset selection; P2 adds explicit camera stacking; P3 adds ordered HDR effects.
Editor authoring remains unimplemented.

The RenderPipeline example performs a real compute, offscreen graphics, and
sampled presentation chain. `UpdateColorPass` writes a storage buffer,
`GeometryPass` draws into an offscreen target, and `PresentPass` samples that
target into presentation. Debug/Release 120-frame runs passed on OpenGL,
Vulkan, and D3D12 with the frame-119 storage-buffer readback and normal
shutdown. D3D12 used the debug layer; its sampler-creation warnings are benign
and existing.

Lifecycle follow-up verification used an isolated GE_WITH_MONO=OFF CMake build:
Debug and Release engine/example builds and 120-frame OpenGL, Vulkan shared,
D3D12 and Vulkan dedicated runs passed. Debug additionally passed 6,000 frames
per variant. Each run checked three graph generations, paired frame hooks,
configuration failure recovery, dimensions and compute results. D3D12 debug
layer logs had existing sampler warnings, no errors; Debug Vulkan had no
validation errors or validation-unavailable fallback. This does not verify
script integration, interactive window resize, visible/final pixel output or
long-run memory. D3D12 debug-layer handle growth remains an independent open
diagnosis; the bounded run does not attribute allocation ownership.

## Planned extensions

[Pipeline TODO P1-P3](../../../TODO/Rendering/RenderPipeline.md#planned-pipeline-extensions-2026-10-31)
owns the planned asset/switching, camera stacking and post-processing milestones.
The intended flow is asset validation and pipeline selection between complete
frames, explicit base/overlay composition, then ordered effects before final
presentation. Failed selection must preserve the working pipeline; outgoing
GPU resources remain retained through every relevant queue's completion.
Camera attachment load/store/depth dependencies and effect input/output textures
must be declared through RenderGraph, with explicit unsupported capabilities.
P1/P2/P3's bounded architecture is described above; no
resource pooling, native API coupling or per-frame device wait is introduced.
