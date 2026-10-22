// Preserve the legacy gizmo snapshot while adapting removed ImGui entry points.
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
namespace ImGui
{
    inline void CaptureMouseFromApp(bool capture = true)
    {
        SetNextFrameWantCaptureMouse(capture);
    }
}
#define AddPolyline(points, count, color, closed, thickness) \
    AddPolyline(points, count, color, static_cast<float>(thickness), \
        static_cast<ImDrawFlags>((closed) ? ImDrawFlags_Closed : ImDrawFlags_None))
#include "../../../GEngine-ThirdParty/ImGuizmo/ImGuizmo.cpp"
#undef AddPolyline
