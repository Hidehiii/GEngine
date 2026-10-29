#pragma once
#include "GEngine/Asset/GAsset.h"
#include "GEngine/Runtime/ResourcePack.h"
#include "stb_image.h"
#include <limits>
#include <stdexcept>

namespace GEngine
{
    // stb owns the decoded pixels until the existing upload path frees them.
    inline stbi_uc* LoadResourceImage(const char* path, int* width, int* height,
        int* channels, int desiredChannels)
    {
        const auto bytes = LoadGAsset(std::filesystem::u8path(path), "texture").Payload;
        if (bytes.size() > static_cast<std::size_t>((std::numeric_limits<int>::max)()))
            throw std::runtime_error("Image exceeds decoder input limits.");
        auto* pixels = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(bytes.data()),
            static_cast<int>(bytes.size()), width, height, channels, desiredChannels);
        if (!pixels) throw std::runtime_error(std::string("Cannot decode resource image: ") + path);
        return pixels;
    }
}
