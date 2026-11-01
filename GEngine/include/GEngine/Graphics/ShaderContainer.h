#pragma once
#include <cstdint>
#include <cstring>
#include <vector>
#include <string_view>
#include "GEngine/Graphics/GraphicsCommon.h"

namespace GEngine
{
    struct ShaderContainerPass
    {
        RenderState State;
        std::unordered_map<std::string, std::string> EntryPoints;
        std::string Program;
        bool MeshWorkflow = false;
    };

    struct ShaderContainer
    {
        std::string Name;
        std::vector<ShaderContainerPass> Passes;
    };

    // CPU-only: returns a complete owned snapshot or throws std::invalid_argument.
    GENGINE_API ShaderContainer ParseShaderContainer(std::string_view source,
        std::string_view sourceName = "<shader>");
}
