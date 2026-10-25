# Graphics and compute API

Editor E0.2 uses existing RenderGraph, CopyTexture2DToArrayLayer and
Texture2DArray::ReadLayerData signatures without a new public API. OpenGL
framebuffer color textures now report their real dimensions and RGBA8 format,
allowing the existing validated texture-array copy path. Other formats remain
subject to copy/readback capability checks; no unsupported format is coerced.
Vulkan backend attachment wrappers accept optional width/height metadata and
remain non-owning; this backend constructor change does not affect common engine
interfaces. Vulkan color attachments now support copy-source usage.
Color-only Vulkan render passes omit depth references; default color dependencies
use color-write access. RenderPassSpecification signatures remain unchanged.

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
