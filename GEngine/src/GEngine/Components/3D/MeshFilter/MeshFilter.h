#pragma once
#include "GEngine/Core/Core.h"
#include "GEngine/Components/Component.h"
#include "GEngine/Asset/Mesh.h"


namespace GEngine
{
	class GENGINE_API MeshFilter : public Component
	{
	public:
		MeshFilter() = default;
		MeshFilter(const MeshFilter&) = default;

		void SetMesh(const Mesh& mesh);
		Mesh& GetMesh() { return m_Mesh; }
	private:
		Mesh m_Mesh;
	};
}


