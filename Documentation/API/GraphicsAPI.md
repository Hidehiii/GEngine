# Graphics and compute API

[API index](README.md) · [Rendering architecture](../Architecture/Rendering.md) ·
[Render graph API](RenderGraph.md)

This page is the entry point for the backend-neutral graphics and compute
surface. Public headers are authoritative for signatures; planned features
remain in TODO and must not be described as implemented. Follow the
[Standard workflow](../Workflow/StandardWorkflow.md) when changing this API or
its architecture.

## Topic index

- [Diagnostics and device-owned creation](Graphics/Diagnostics.md)
- [Device capabilities](Graphics/Device.md)
- [Application and layer](Graphics/Application.md)
- [Shader asset](Graphics/ShaderAsset.md)
- [Minimal draw](Graphics/ApplicationDraw.md)
- [Off-screen command submission](Graphics/Commands.md)
- [Resources and shader cache](Graphics/Resources.md)

## Device-owned creation

`Graphics::GetRenderDevice()` creates shaders, materials, graphics and compute
pipelines, samplers, storage buffers and images, 2D/array/cube textures, and
combined texture/sampler wrappers. Native backend selection lives in backend
implementations, not in these resources' static factory functions.
