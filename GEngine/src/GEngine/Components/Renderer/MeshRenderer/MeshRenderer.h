#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Components/Component.h"
#include "GEngine/Renderer/VertexArray.h"
#include "GEngine/Renderer/Shader.h"
#include "GEngine/Renderer/RenderBuffer.h"
#include "GEngine/Renderer/Material.h"

namespace GEngine
{

	// Contains the mesh renderer of a game object
	// Renders the mesh of the game object
	class GENGINE_API MeshRenderer : public Component
	{
	public:
		MeshRenderer() = default;
		MeshRenderer(const MeshRenderer&) = default;

		void Init();

		void OnRender();

		void SetMaterial(Ref<Material> material);
	public:
		Ref<Material> m_Material;
	private:
		Ref<VertexArray> m_VertexArray;
		Ref<VertexBuffer> m_VertexBuffer;
	};
}


