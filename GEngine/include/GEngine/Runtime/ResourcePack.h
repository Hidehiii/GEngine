#pragma once
#include "GEngine/Core/Core.h"
#include <filesystem>
#include <vector>

namespace GEngine
{
    // Validate and mount the executable-adjacent .gpak before application construction.
    // Returns empty for ordinary development executables without a pack.
    GENGINE_API std::filesystem::path PreparePackagedResources(bool required = false);
    // Mount atomically after complete validation. Existing reads retain their archive.
    GENGINE_API void MountResourcePack(const std::filesystem::path& pack);
    GENGINE_API bool HasResourcePack();
    // Returned bytes own their storage; no borrowed archive or upload pointers.
    GENGINE_API std::vector<char> ReadResource(const std::filesystem::path& path);
    GENGINE_API bool ResourceExists(const std::filesystem::path& path);
    GENGINE_API bool ResourceIsDirectory(const std::filesystem::path& path);
    GENGINE_API std::vector<std::filesystem::path> ListResourceDirectory(const std::filesystem::path& path);
    // Reject writes to the archive namespace; development writes are unchanged.
    GENGINE_API void EnsureResourceWritable(const std::filesystem::path& path);
}
