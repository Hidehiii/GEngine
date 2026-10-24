#pragma once

#include <GEngine/Math/Math.h>

namespace GEngine::Editor
{
    enum class GizmoOperation { None = -1, Translate, Rotate, Scale };
    enum class GizmoSpace { Local, World };

    struct GizmoResult
    {
        bool Changed = false;
        bool Hovered = false;
        bool Active = false;
    };

    class GizmoController
    {
    public:
        static void BeginFrame();
        static void SetViewport(Vector2 position, Vector2 size, bool orthographic);
        static GizmoResult Manipulate(const Matrix4x4& view, const Matrix4x4& projection,
            GizmoOperation operation, GizmoSpace space, Matrix4x4& transform,
            const Vector3* snap = nullptr);
        static void DrawGrid(const Matrix4x4& view, const Matrix4x4& projection,
            const Matrix4x4& transform, float size);
    };
}
