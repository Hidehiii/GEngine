#include "GizmoController.h"
#include <imgui.h>
#include <ImGuizmo.h>
#include <stdexcept>

namespace GEngine::Editor
{
    void GizmoController::BeginFrame()
    {
        ImGuizmo::SetImGuiContext(ImGui::GetCurrentContext());
        ImGuizmo::BeginFrame();
    }

    void GizmoController::SetViewport(Vector2 position, Vector2 size, bool orthographic)
    {
        ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
        ImGuizmo::SetRect(position.x, position.y, size.x, size.y);
        ImGuizmo::SetOrthographic(orthographic);
    }

    GizmoResult GizmoController::Manipulate(const Matrix4x4& view, const Matrix4x4& projection,
        GizmoOperation operation, GizmoSpace space, Matrix4x4& transform, const Vector3* snap)
    {
        if (operation == GizmoOperation::None) return {};
        ImGuizmo::OPERATION nativeOperation;
        switch (operation)
        {
        case GizmoOperation::Translate: nativeOperation = ImGuizmo::TRANSLATE; break;
        case GizmoOperation::Rotate: nativeOperation = ImGuizmo::ROTATE; break;
        case GizmoOperation::Scale: nativeOperation = ImGuizmo::SCALE; break;
        default: throw std::invalid_argument("Unknown gizmo operation");
        }
        ImGuizmo::MODE nativeSpace;
        switch (space)
        {
        case GizmoSpace::Local: nativeSpace = ImGuizmo::LOCAL; break;
        case GizmoSpace::World: nativeSpace = ImGuizmo::WORLD; break;
        default: throw std::invalid_argument("Unknown gizmo space");
        }
        const bool changed = ImGuizmo::Manipulate(Math::ValuePtr(view), Math::ValuePtr(projection),
            nativeOperation, nativeSpace, Math::ValuePtr(transform), nullptr,
            snap ? Math::ValuePtr(*snap) : nullptr);
        return { changed, ImGuizmo::IsOver(), ImGuizmo::IsUsing() };
    }

    void GizmoController::DrawGrid(const Matrix4x4& view, const Matrix4x4& projection,
        const Matrix4x4& transform, float size)
    {
        ImGuizmo::DrawGrid(Math::ValuePtr(view), Math::ValuePtr(projection), Math::ValuePtr(transform), size);
    }
}
