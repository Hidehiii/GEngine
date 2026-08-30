#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Components/Component.h"
#include "GEngine/Asset/Mesh.h"


namespace GEngine
{
	// Contains the mesh filter of a game object
	// Contains the mesh of the game object
	class GENGINE_API MeshFilter : public Component
	{
	public:
		MeshFilter() = default;
		MeshFilter(const MeshFilter&) = default;

		void SetMesh(const Mesh& mesh);
		const Mesh& GetMesh() { return m_Mesh; }

		operator bool() const { return m_Mesh; }
	private:
		// Mesh of the game object
		Mesh m_Mesh;

		friend class MeshRenderer;
	};
}


