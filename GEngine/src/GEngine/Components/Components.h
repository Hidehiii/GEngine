#pragma once
#include "GEngine/Components/Attribute/Attribute.h"
#include "Transform/Transform.h"
#include "Renderer/ImageRenderer/ImageRenderer.h"
#include "Name/Name.h"
#include "Camera/Camera.h"
#include "NativeScript/NativeScript.h"
#include "2D/BoxCollider2D/BoxCollider2D.h"
#include "2D/RigidBody2D/RigidBody2D.h"
#include "ID/ID.h"
#include "Renderer/CircleRenderer/CircleRenderer.h"

namespace GEngine
{
	template<typename... Component>
	struct ComponentGroup
	{
	};


	// expect id and name to be in every component group
	using AllComponents =
		ComponentGroup<Transform, ImageRenderer,
		Camera, NativeScript, RigidBody2D, BoxCollider2D,
		Attribute, CircleRenderer>;
}
