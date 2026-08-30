#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Components/Component.h"
#include "GEngine/Graphics/GraphicsPipeline.h"
#include "GEngine/Asset/Mesh.h"
namespace GEngine
{

	// Contains the mesh renderer of a game object
	// Renders the mesh of the game object
	class GENGINE_API MeshRenderer : public Component
	{
	public:
		MeshRenderer() = default;
		MeshRenderer(const MeshRenderer&) = default;

		void OnRender();

		void SetMaterial(Ref<Material> material);
		Ref<Material> GetMaterial() { return m_Material; }
	private:
		void UpdatePipeline();
	private:
		Ref<GraphicsPipeline>	m_Pipeline = nullptr;
		Ref<Material>			m_Material = nullptr;
		friend class MeshFilter;
	};
}


