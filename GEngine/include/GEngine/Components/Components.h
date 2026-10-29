#pragma once
#include "GEngine/Object/Transform.h"
#include "Camera/CameraComponent.h"
#include "StaticMesh/StaticMeshComponent.h"
#include "Image/ImageComponent.h"
namespace GEngine
{
    template<typename... T> struct ComponentGroup {};
    using AllComponents = ComponentGroup<StaticMeshComponent, ImageComponent, CameraComponent>;
}
