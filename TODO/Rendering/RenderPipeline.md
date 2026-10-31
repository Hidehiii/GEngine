# Render pipeline framework

## First composable pipeline milestone

- [x] Rewrite the abandoned two-method `RenderPipeline` abstraction into a
      backend-neutral composable render pipeline framework, and add an example
      that actually uses it. Scope: replace the old
      `GEngine/Graphics/RenderPipeline.h` contract with a new renderer-owned
      pipeline/pass API under `GEngine/Renderer`; support ordered pass objects,
      a graph-facing builder, execution against `RenderGraph`, and an explicit
      presentation phase. Add a focused example that performs a real
      compute -> offscreen graphics -> sampled presentation chain on OpenGL,
      Vulkan, and D3D12. Do not claim full Unity URP/HDRP feature parity,
      asset-driven pipeline switching, camera stacking, post-processing stacks,
      or editor pipeline authoring in this milestone.
      Acceptance: the public API compiles in Debug/Release; the example builds
      and runs on all three backends for 120 frames; the offscreen result is
      numerically verified or visibly sampled into the final presentation path;
      pass ordering and present execution are exercised; architecture/API
      example documentation and this TODO are synchronized.
      Implemented: the old `GEngine/Graphics/RenderPipeline.h` contract is
      removed. `GEngine/Renderer/RenderPipeline.h` now provides
      `RenderPipelinePass`, `RenderPipelineBuilder`, `RenderPipelineContext`,
      and a composable `RenderPipeline` that owns a persistent `RenderGraph`.
      Derived pipelines create resources/targets and add ordered passes in
      `Configure`; `Execute` runs per-frame hooks and graph submission, while
      `Present` runs explicit presentation hooks. A focused
      `Example/RenderPipeline` implements `UpdateColorPass`,
      `GeometryPass`, and `PresentPass`, producing a real compute, offscreen
      graphics, and sampled presentation chain with a frame-119 numeric readback.
      Verification: CMake Debug/Release builds passed. OpenGL, Vulkan, and
      D3D12 each completed 120-frame Debug/Release runs; every run passed the
      storage-buffer readback and normal shutdown. D3D12 used the debug layer.
      Premake VS2022 regeneration and the Debug RenderPipeline build also
      passed. Premake runtime and Vulkan dedicated queues are not part of this
      first milestone. Remaining: manual visible pixel checks, pipeline
      assets/switching, camera stacking, post-processing stacks, editor
      authoring, resize/long-run behavior, and broader URP/HDRP-style features
      remain open.
