## Device capabilities

`Graphics::GetCapabilities()` returns a backend-specific snapshot derived from
device queries rather than API identity. The result includes the backend name,
queried version and the concrete query sources used to derive the feature
booleans.

```cpp
const auto capabilities = Graphics::GetCapabilities();
GE_INFO("Backend {} version {}.", capabilities.Backend, capabilities.Version);
for (const auto& source : capabilities.QuerySources)
    GE_INFO("Capability source: {}.", source);
```

OpenGL uses its core version and enumerated extensions. Vulkan queries physical
device properties/features, device extensions, format properties and queue
families. D3D12 queries feature levels, options, root-signature and shader-model
support plus the format support used by render targets and storage images.
Capability reporting stays per-device; one backend never infers support from
another.

The graphics sections below describe the common immediate rendering path used
by `Example/Triangle`. Include `<GEngine.h>` in an application project, and add
specific headers for APIs outside that umbrella.
