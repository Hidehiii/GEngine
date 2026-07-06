#pragma once
#include "GEngine/Graphics/Material.h"

namespace GEngine
{
	class GENGINE_API D3D12Material : public Material
	{
	public:
		D3D12Material(const Ref<Shader>& shader, const std::string& name = "");
	};
}


