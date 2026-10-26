#pragma once
#include "GEngine/Core/Core.h"
#include <filesystem>

namespace GEngine
{
    // Validate/extract the executable-adjacent .gpak before application construction.
    // Returns empty for ordinary development executables without a pack.
    GENGINE_API std::filesystem::path PreparePackagedResources(bool required = false);
}
