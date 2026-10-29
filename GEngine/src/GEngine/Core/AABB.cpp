#include "GEpch.h"
#include "GEngine/Core/AABB.h"
#include "GEngine/Components/Components.h"
namespace GEngine
{
    void AABB::CalculateAABB(GameObject& object)
    {
        m_Min = m_Max = object.GetTransform().GetPosition();
        const auto* component = object.TryGetComponent<StaticMeshComponent>();
        if (!component || !component->GetMeshAsset()) return;
        const auto mesh = component->GetMeshAsset();
        const auto model = object.GetTransform().GetModelMatrix();
        bool first = true;
        for (const auto& vertex : mesh->m_Vertices)
        {
            const Vector3 position = model * vertex.m_Position;
            if (first) { m_Min = m_Max = position; first = false; }
            else
            {
                m_Min = {std::min(m_Min.x, position.x), std::min(m_Min.y, position.y), std::min(m_Min.z, position.z)};
                m_Max = {std::max(m_Max.x, position.x), std::max(m_Max.y, position.y), std::max(m_Max.z, position.z)};
            }
        }
    }
}
