# RenderPipeline

## Shader parser rollback checks

After a normal engine/example build, run:

```bat
python -B Tests/RenderPipelineAssets.py dist/pipeline-lifecycle/windows-x86_64/Debug/RenderPipeline/RenderPipeline.exe --parser --logs build/shader-parser/Debug
```

Repeat with Release and individual --backend choices. This opt-in extends P1
with an invalid DepthWrite token and a missing container brace, each retried
twice. Require `Shader parser rollback passed: 4 cases.`, 22 total rejections,
unchanged active instance/settings, no cache folders for either malformed shader,
and the existing valid-output switch/readback/resize/discard/shutdown checks.
Failure indicators include assertion termination, cache artifacts, missing parse
diagnostics, changed active pipeline or numeric/native diagnostics. Fresh copied
run directories avoid stale fixture cache. Direct launch uses
GENGINE_RENDER_PIPELINE_ASSET_CHECK=1 and GENGINE_RENDER_PIPELINE_PARSER_CHECK=1
with a frame limit. Existing modes are unchanged.
CPU parser tests pass independently, but this runtime mode has not been run:
the current host's known D3D12 version intentionally blocks complete builds.
No OpenGL/Vulkan/D3D12 runtime success is inferred from CPU checks. Follow the
[version guide](../../Documentation/Guides/BuildAndRun.md#changing-sdkruntime-versions).

## Ordered HDR post-processing (P3)

Build the native example and run from the repository root:

```bat
python -B Tests/RenderPipelineAssets.py dist/pipeline-lifecycle/windows-x86_64/Debug/RenderPipeline/RenderPipeline.exe --post --logs build/post-process/Debug
```

Repeat with Release; use `--frames 6000` for bounded stress or `--backend` to
select OpenGL, Vulkan, D3D12 or VulkanDedicated. The runner sets
GENGINE_RENDER_PIPELINE_POST_CHECK=1, isolates other modes, uses 12 command slots
in copied Config.ini and copies assets/logs/CSV without changing local selections.
Direct launch requires this flag, a frame limit and CommandBufferCount >= 9:
two camera commands, two effect commands, a final sampled reader and completion,
plus three readback graph commands on diagnostic frames. POST takes precedence
over STACK/ASSET; no flags preserves the original C++ mode.

PostProcess.gasset uses payload v3: a Base and top-right UI camera compose linear
RGBA16F HDR, then GEngine.PostProcess.v1 processes the completed composition
once into RGBA8 Output before presentation. Base RGBA is (4,.5,.125,.5), UI is
(.25,2,.002,.75), including RGB above 1 and a dark component for the sRGB toe.
The example deploys the explicit shader v1 contract from Shaders/PostProcess.gasset;
the engine prepares independent materials and graph-owned HDR intermediates.
It never uses compute or native backend identity branches.

Every 15 frames the example exercises: global bypass; exposure only at -2 EV;
Reinhard only; exposure +1 EV then Reinhard; reversed order with -1 EV;
live -16 EV exposure with tone disabled and sRGB encoding; an empty stack;
and a two-effect sRGB chain. Frames 1/2 additionally toggle settings on the same
instance, including +16 EV finite-half clamping. Case 4 resizes both targets to
138x86; cases 6/7 replace empty/two-stage topology. Other cases begin at 96x80.
Cases with topology change use prepared P1 candidates, preserving old selection
on failure. The visible colors/brightness change between cases; the top-right
UI remains in the same place. Alpha remains part of the output and presentation
blends it, so numeric output is verified before window blending.

Each 120-frame run requires 18 invalid-candidate checks, four invalid live
snapshots rejected before any camera draw, two same-instance live markers,
six replacements, eleven CPU-reference quadrant probes and normal shutdown.
`Post-process pass reuse passed` additionally checks repeated preparation of an
equivalent graph with the same pass, without accumulating old stage resources.
The reference computes exposure/Reinhard/encoding independently in double
precision; tolerance is two RGBA8 units for half precision and quantization.
Each frame checks exact expected target count (2 for zero/one effect, 3 for two)
and unchanged framebuffer identities between replacements. 6000 frames require
349 replacements and 403 probes. Diagnostic readbacks synchronize explicitly;
the implementation adds no per-frame device wait. Native diagnostics, mismatch,
missing markers, unexpected allocation or timeout fail the runner. Manual final
window pixels, OS display encoding, export dependency discovery and device-loss
injection are separate. Debug/Release builds and 120-frame runs passed all four
variants including the pass reuse regression; final logs:
build/post-process/final-{Debug,Release}. Debug also passed 6000 frames per variant
in build/post-process/final-stress-Debug, with clean native diagnostics. P1/P2 regressions
(p1-/p2-{Debug,Release}), original C++ mode (fresh 124-frame directories) and
rebuilt MSAA regression (msaa-{Debug,Release}) passed all four variants in both
configurations. See the P3 TODO for bounded memory samples and untested cases,
including three/four-slot chains, numeric HDR MSAA and Mono/Premake/archive runtime.

## Camera stacking (P2)

Set `GENGINE_RENDER_PIPELINE_STACK_CHECK=1` and
`GENGINE_RENDER_PIPELINE_FRAME_LIMIT=120`, or run from the repository root:

```bat
python -B Tests/RenderPipelineAssets.py dist/pipeline-lifecycle/windows-x86_64/Debug/RenderPipeline/RenderPipeline.exe --stack --logs build/camera-stacks/Debug
```

Use the actual native executable path; repeat with Release and optionally
`--frames 6000`. `--backend` selects one of the four backend variants. The runner
copies config/assets and records logs/memory without changing backend selections.
Stack mode takes precedence over asset mode; without either flag the original
C++ pipeline remains the default. CMake/Premake source globs already cover the
new helper and engine CameraStack files (regenerate Premake before building).

CameraStack.gasset uses payload v2 and registers the reusable CameraStackPass.
The application binds Base/Scene/UI IDs to three caller-owned orthographic cameras.
Draw callbacks use their projection matrices and independent camera materials.
Passes compose into one Output texture; a fullscreen diagnostic samples it into
Presented, then presentation samples Presented once. Every
15 frames a new P1 candidate exercises one of eight cases: retained depth occludes
the farther blue camera; depth clear exposes blue; disabled UI; reversed overlay
order; whole-target color clear with bottom-left blue/top-right green viewports;
inactive base clears everything; inactive parent suppresses its missing child;
and 138x86/4x/D32 replacement. Other cases use 96x80/1x/D24S8, with selected
4x cases. Cases requiring 4x fail explicitly when device queries reject support.

Expected output normally has a red base and green top-right UI. Clear-depth
variants expose blue; disabled cases suppress overlays; the color-clear case has
black outside the two overlay rectangles. Numeric checks sample all four quadrants
of both raw and sampled outputs and assert recorder order. AttachmentUvStartsAtTop
selects sampling UVs and adapts native readback rows without backend identity
checks; renderer viewport/scissor coordinates remain uniformly top-left. Clears are
whole attachment clears, not viewport clears. No actual window interaction is
automated; final-window/presentation pixels remain a separate manual check.

Each 120-frame run must report `Camera stack preflight passed: 16 cases`, three
binding rejections before recording, two live binding changes on the same pipeline,
seven replacements, eleven paired-output pixel probes and
normal shutdown. Preflight covers cycles, duplicate/shared IDs, missing/disconnected
nodes, extra bases, viewport/depth/target/sample mismatch and v1 incompatibility.
Runtime checks cover missing/null/duplicate active bindings and invalid command
viewports. Disabled bindings/parents may omit descendants; active cameras may not
be missing. Borrowed camera pointers live through Execute, not GPU completion.
Callbacks must use separate per-camera parameter storage. Existing P1 and default
modes must pass regression. `--frames 6000` requires 399 replacements and 403
paired-output probes. Final-source Debug/Release GE_WITH_MONO=OFF builds and
120-frame GL, Vulkan shared, D3D12 layer-on and Vulkan dedicated runs passed;
logs: build/camera-stacks/final-{Debug,Release}. Debug also passed 6000 frames
per variant; logs/memory CSV: build/camera-stacks/final-stress-Debug. Native
diagnostics were clean. P1 regression (final-p1-{Debug,Release}), original C++
mode (fresh 123-frame directories) and rebuilt FrameGraphTriangle --msaa
(final-msaa-{Debug,Release}) passed all four variants in both configurations.
See the P2 TODO for measured memory and untested cases. Manual final-window
pixels, Mono/Premake runtime integration, archive/export discovery and injected
device failures remain unverified. The separate sampled target verifies texture
consumption numerically; it is not a post-processing effect stack.

## Pipeline asset selection (P1)

Set `GENGINE_RENDER_PIPELINE_ASSET_CHECK=1` and
`GENGINE_RENDER_PIPELINE_FRAME_LIMIT=120`, or run from the repository root:

```bat
python -B Tests/RenderPipelineAssets.py dist/pipeline-lifecycle/windows-x86_64/Debug/RenderPipeline/RenderPipeline.exe --logs build/pipeline-assets/Debug
```

Use the actual native executable path and repeat with Release. The runner copies
assets/config into isolated directories, enables native diagnostics, checks exact
markers, and records runtime.log/memory.csv. `--backend` selects OpenGL, Vulkan,
D3D12 or VulkanDedicated; `--frames 6000` exercises 399 replacements. A timeout
stops only its own process. The original C++ lifecycle example remains the default
when the asset flag is absent. CMake CONFIGURE_DEPENDS and Premake src/**.cpp globs
cover the new files; regenerate Premake before building this mode.

Warm.gasset and Cool.gasset define ordered `Example.SolidColor.v1` and
`Example.Present.v1` passes. Their registered C++ validators/factories consume
scalar settings and shader references. Expected output is a warm red or cool blue
square, switching every 15 frames. Warm uses 96x64/1x/D24S8, cool 64x96/4x/D32;
from frame 60 warm candidate targets resize to 137x83. The cool fixture requires
queried 4x color/depth support and rejects unsupported devices rather than
downgrading. This is a regression fixture, not a built-in engine render pipeline.

Acceptance checks: GAsset and payload round trips; 18 failed candidates preserve
the active pipeline/settings; factories are not called for preflight failures;
configuration, preparation, graph-cycle, null-factory and invalid-HLSL compilation
errors roll back; hooks
and Execute/Present gaps reject selection. Target-dependent presentation bindings
are created in Prepare. Caller camera vector identity and reset frame indices
are checked. Every activation verifies RGBA pixels at center/corner through a
transfer readback and graphics read join. An unused outgoing target is recorded
before replacement and alternately submitted or discarded/reset after old CPU
pipeline destruction, exercising
native retention. LifetimeProbe is intentionally not sampled by presentation.
OpenGL records immediately: wrapper reset does not undo already issued GL work.
An additional probe replaces the source registry during validation and still
prepares a candidate from its factory snapshot.

Expected markers: `Pipeline asset preflight and rollback passed`, 18
`Pipeline asset rejection passed`, seven `Pipeline asset switch passed` and nine
`Pipeline asset pixels passed` in 120 frames, followed by normal shutdown.
Pixel mismatch, missing markers, native warnings/errors or timeout fail the run.
The invalid-HLSL fixture emits an expected compiler error; it must be followed
by the shader-compilation rejection marker and successful active-pipeline pixels.
Other malformed legacy shader-container directives are not covered by P1.
Readbacks are diagnostic synchronization; no per-frame device idle is added.
Final P1 CMake GE_WITH_MONO=OFF Debug/Release builds and 120-frame runs passed
GL, Vulkan shared, D3D12 layer-on and Vulkan dedicated separately, with clean
native diagnostics. Logs: build/pipeline-assets/final-{Debug,Release}. Debug also
passed 6000 frames per variant (399 switches, 401 probes, 200 outgoing submissions
and 199 discards); logs/CSV: build/pipeline-assets/final-stress-Debug. Original
C++ mode passed again in both builds. See the P1 TODO for measured memory and
verification limits. Manual final-window pixels, automatic export
dependency discovery, editor authoring, camera stacking and post-processing are
outside P1. The development round-trip file is PipelineAssetRoundtrip.gasset in
the isolated run directory; archive writes remain explicitly rejected.

This example uses the new composable `RenderPipeline` and `RenderPipelinePass`
APIs. It performs a real compute → offscreen graphics → sampled presentation
chain:

1. `UpdateColorPass` toggles a storage-buffer color on the compute queue.
2. `GeometryPass` draws a triangle into an offscreen RGBA8 target.
3. `PresentPass` samples that target and presents it through the portable
   graphics presentation path.

The example is backend-neutral. Select `GraphicsAPI: 1`, `2`, or `3` in
`Config.ini` and run it from this directory. Set
`GENGINE_RENDER_PIPELINE_FRAME_LIMIT=120` for a bounded run. Expected logs:

- `RenderPipeline initialized: compute, offscreen geometry, sampled presentation.`
- `RenderPipeline configuration failure recovery passed.`
- `RenderPipeline external pass configured; mutation guards passed.`
- `RenderPipeline rebuilt offscreen target at frame 30.` (and 60)
- `RenderPipeline compute readback passed at frame 29 (512x512).`
- `RenderPipeline compute readback passed at frame 59 (256x384).`
- `RenderPipeline compute readback passed at frame 119 (640x360).`
- `RenderPipeline completed 120 frames; requesting normal shutdown.`

The storage-buffer color is checked before and after repeated explicit Reset
and offscreen target rebuilds. Target dimensions are checked every frame.
An externally registered pass verifies central configuration, matching
Execute/Present frame numbers, reset frame numbering, reentrancy rejection and
invalid mutation/phase rejection. A deliberate initial Configure exception
checks that the failed generation requires Reset and can then recover.
These checks execute in both Debug and Release, independently of assertions.
Any unexpected acceptance, frame mismatch, dimension or color mismatch throws
and fails the run. AddPass/RemovePass apply only before configuration; the
pipeline centrally invokes each pass Configure in order.

The final visible result is the offscreen target sampled by `PresentPass`.
Expect a centered triangle alternating red intensity within the displayed quad;
it remains visible after the offscreen aspect/size changes. Black output,
stale images after rebuild, flicker or debug-layer lifetime errors are failures.
For manual verification run without the frame limit, resize the actual window,
minimize/restore it and close normally after the two rebuilds. These interactive
checks and long-run memory checks are separate from bounded offscreen rebuilds.
The default C++ mode does not load assets; the P1 opt-in mode above validates
explicit asset switching. Neither mode verifies final-present pixels. Camera
stacking and post-processing are exercised by the separate P2/P3 modes above;
editor pipeline authoring remains unimplemented.

## Lifecycle regression evidence (2026-10-31)

Isolated CMake VS2022 Debug/Release engine and example builds passed with
`GE_WITH_MONO=OFF` (this example disables scripting). OpenGL, Vulkan shared,
D3D12 and Vulkan dedicated each passed 120 frames in Debug and Release;
Debug also passed 6,000 frames per variant. All runs checked three successful
configurations, both target rebuilds, three readbacks and normal exit 0.
Set `GENGINE_VULKAN_DEDICATED_QUEUES=1` for the dedicated Vulkan variant;
set `GENGINE_D3D12_DEBUG_LAYER=1` to enable D3D12 validation in Release too.
Run from a copy of this directory to keep backend settings and shader caches
isolated. Logs and sampled process memory are under
`build/pipeline-lifecycle/runs/<configuration>-<backend>-<frames>/` at repo root.
Existing D3D12 sampler and OpenGL readback performance warnings were observed;
no validation errors were logged. D3D12 debug-layer handle growth remains
unattributed. This short extension does not establish long-run memory stability.
The repository Mono SDK was unavailable, so script-enabled integration and
Premake were not revalidated. Actual window/visible checks remain unverified;
desktop automation is excluded at the user's request.
