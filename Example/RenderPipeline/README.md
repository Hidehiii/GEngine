# RenderPipeline

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
No final-present pixel readback or automatic pipeline-asset switching is claimed.
This is a first composable-pipeline milestone, not full URP/HDRP parity:
camera stacking, post-processing stacks, pipeline assets, and editor pipeline
authoring are not implemented.

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
