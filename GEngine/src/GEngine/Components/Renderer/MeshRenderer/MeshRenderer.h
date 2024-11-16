#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Components/Component.h"
#include "GEngine/Renderer/Pipeline.h"
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
		Ref<Material> GetMaterial() { return m_Pipeline ?  m_Pipeline->GetMaterial() : nullptr; }
	private:
		void UpdateMesh(const Mesh& mesh);
	private:
		Ref<Pipeline>	m_Pipeline = nullptr;
		friend class MeshFilter;
	};
}


