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
