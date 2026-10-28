#pragma once
#include "GEngine/Core/Core.h"

namespace GEngine
{
    class Scene;
    class CommandBuffer;

    // Records supported active scene components into the caller's graphics pass.
    class GENGINE_API SceneRenderer
    {
    public:
        static void Render(Scene& scene, const Ref<CommandBuffer>& command);
    };
}
