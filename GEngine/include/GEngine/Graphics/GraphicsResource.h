#pragma once

#include "GEngine/Core/Core.h"
#include "GEngine/Graphics/GraphicsCommon.h"

namespace GEngine
{
	class GraphicsAPI;

	// The renderer and frame graph use this opaque engine resource instead of
	// passing backend handles through their public API.  Native access is only
	// consumed by a GraphicsAPI implementation.
	class GENGINE_API GraphicsResource
	{
	public:
		virtual ~GraphicsResource() = default;
		virtual GraphicsResourceType GetResourceType() const = 0;
		virtual bool RequiresExplicitStateTransition() const { return true; }

	private:
		// Native handles are intentionally private to the device abstraction.  A
		// render feature may declare a resource dependency, but it cannot couple
		// that declaration to a backend handle type.
		virtual void* GetNativeResource() const = 0;
		friend class GraphicsAPI;
	};
}
