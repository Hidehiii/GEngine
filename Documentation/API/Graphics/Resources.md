## Resources

- `Texture2D::Create`, `CubeMap::Create`, and texture-array APIs create sampled image resources.
- `UniformBuffer` and `Material` hold shader properties and constant-buffer data.
- `Sampler` and combined texture/sampler types provide sampling state.
- `StorageBuffer`, `StorageImage2D`, and `ComputePipeline` provide compute resources and dispatch.
- `FrameBuffer` and `RenderPass` create off-screen rendering targets.

Use `Material::Set*` APIs that correspond to reflected shader property names. Recreate or update the material's resources after changing the shader interface.
No public resource signature changes for VMA. On the Vulkan backend, buffers,
images, upload staging, and readback staging are allocated through VMA and
retired with their native objects after GPU completion. Swap-chain images and
non-memory Vulkan objects remain backend-owned native handles.

## Shader cache

Compiled shader data is stored under the directory configured by `ShaderCacheDirectory` in `Config.ini`. The cache is keyed by source hash and backend. Reflection still runs when cached DXIL/SPIR-V is loaded, so engine-side reflection fixes do not require editing the shader source.

D3D12 texture arrays (2026-11-01) explicitly allocate the same single mip that
their public metadata and layer-copy/readback indexing report. This corrects
layer 1 being mistaken for mip 1 on textures larger than 1x1; the existing
texture-array signatures and one-mip contract are unchanged. The MSAA MRT
regression exercises this path at 31x29 and 47x33 with both array layers.

Framebuffer formats outside the existing RenderImage2DFormat schema report
NONE through Texture2D::GetFormat instead of an RGBA8 alias or undefined value.
Their native attachment format remains in FrameBufferSpecification and the
per-format device query; shader use follows that native format. RGBA8 array
copy/readback does not accept these differently typed images.
