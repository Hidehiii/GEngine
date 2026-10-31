## Backend layering and shader ownership

Packaged shader cache files live in game-root Cache/Shaders/<API>/ and persist
between launches. Backend directories prevent switching APIs from clearing another
backend's cache. Cache fingerprints include recursively referenced literal HLSL
include contents as well as source and target settings. This CPU file-cache change
does not alter shader module ownership, GPU synchronization or public factories.

Application and renderer code use `GEngine/include/GEngine` interfaces.
`GEngine/src/Platform` and `GEngine/include/Platform` contain OpenGL, Vulkan,
and D3D12 resource, command, context, and presentation implementations;
`Surface` contains GLFW and Win32 windows/input adapters. Native handles,
allocation policies, and synchronization remain behind those boundaries.
Vulkan engine-owned buffers, images, staging uploads, and readbacks use the
context-owned VMA allocator from the official Vulkan SDK. Allocation handles
are backend-private; resource retirement destroys them after submitted GPU
work and before the allocator/device.

OpenGL backend construction selects an API without issuing GL calls. Window
creation makes the context current and loads GLAD; `GraphicsRuntime::Initialize`
then sets up debug output/default GL state. Loader failure throws before a GL
state call, including in Release. OpenGL translates the compiler's Vulkan
SPIR-V intermediate to desktop GLSL 450 using SPIRV-Cross; it does not submit
Vulkan binaries to `glShaderBinary`. Shader compile/link failures throw and
release partial programs. Both vertex-buffer constructors create a VAO and
destruction deletes it; non-indexed draws do not require an index buffer.
Separate Texture2D/cube-map and sampler reflection is translated into combined
image samplers; material code binds both resources to the image unit. Other
complex texture/sampler combinations remain explicitly unsupported until they
gain focused coverage.

`Shader` owns reflection data and compiled bytes. Reflected vertex inputs set
buffer format/offset/stride; D3D12 uses DXIL reflection and Vulkan uses SPIR-V
reflection. `Shader::Create(path)` checks a cache keyed by active API and
lexically normalized absolute path before backend construction. Relative
paths resolve against the working directory; symlinks/case variants are not
canonicalized. `GetShader(name)` is a compatibility lookup, and
`ShutdownCache` clears both caches while the context is alive. Cache lookup
is render-thread-only, runtime-scoped, and not hot reload.
