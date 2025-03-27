#pragma once
#include "GEngine/Graphics/GraphicsPresent.h"

namespace GEngine
{
	class GENGINE_API D3D12GraphicsPresent : public GraphicsPresent
	{
	public:
		D3D12GraphicsPresent();
		virtual bool AquireImage() override;
		virtual void Begin() override;
		virtual void End() override;
	};
}


