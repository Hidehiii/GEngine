#pragma once
#include <GEngine.h>

namespace GEngine
{
	class Hierarchy
	{
	public:
		Hierarchy() = default;
		Hierarchy(const Ref<Scene> scene);

		void SetContext(const Ref<Scene> scene);
		void DrawComponents(GameObject gameObject);

		void OnGuiRender();

		GameObject GetSelectedGameObject() const { return m_SelectionContext; }
		void SetSelectedGameObject(GameObject obj) { m_SelectionContext = obj;  }
	private:
		template<typename... Component>
		auto GetAllGameObjectWithComponents()
		{
			return m_Context->m_Registry.view<Component...>();
		}
	private:
		Ref<Scene> m_Context;
		GameObject m_SelectionContext;
		friend class GEngineEditorLayer;
	};
}


