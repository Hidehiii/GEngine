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
- `RenderPipeline compute readback passed after 119 frames.`
- `RenderPipeline completed 120 frames; requesting normal shutdown.`

At frame 119 the storage-buffer color is read back and checked numerically.
The final visible result is the offscreen target sampled by `PresentPass`.
Manual pixel inspection and interactive resize are not part of the bounded run.
This is a first composable-pipeline milestone, not full URP/HDRP parity:
camera stacking, post-processing stacks, pipeline assets, and editor pipeline
authoring are not implemented.
