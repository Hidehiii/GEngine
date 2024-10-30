#pragma once

// For use by GEngine applications

#include "GEngine/Application.h"
#include "GEngine/Core/Time.h"
#include "GEngine/Core/Log.h"
#include "GEngine/Layer.h"
#include "GEngine/ImGui/ImGuiLayer.h"
#include "GEngine/Core/Input.h"
#include "GEngine/Core/UUID.h"
#include "GEngine/Core/Buffer.h"



// Math 
#include "GEngine/Math/Math.h"
#include "GEngine/Math/Vector2.h"
#include "GEngine/Math/Vector3.h"
#include "GEngine/Math/Vector4.h"
#include "GEngine/Math/Matrix4x4.h"

// Render
#include "GEngine/Renderer/Renderer.h"
#include "GEngine/Renderer/Renderer2D.h"
#include "GEngine/Renderer/RenderCommand.h"
#include "GEngine/Renderer/VertexArray.h"
#include "GEngine/Renderer/RenderBuffer.h"
#include "GEngine/Renderer/Shader.h"
#include "GEngine/Renderer/Pipeline.h"
#include "GEngine/Renderer/Texture.h"
#include "GEngine/Renderer/SubTexture2D.h"
#include "GEngine/Renderer/FrameBuffer.h"
#include "GEngine/Renderer/UniformBuffer.h"
#include "GEngine/Renderer/Material.h"

// Camera
#include "GEngine/Components/Camera/OrthoGraphicCamera.h"
#include "GEngine/Components/Camera/OrthoGraphicCameraController.h"


// Components
#include "GEngine/Components/Components.h"



// Object
#include "GEngine/Object/Scene/Scene.h"
#include "GEngine/Object/GameObject.h"


// Tools
#include "GEngine/Tools/Serialization/Serializer.h"
#include "GEngine/Tools/ModelImporter.h"

// Utils
#include "GEngine/Utils/PlatformUtils.h"
#include "GEngine/Utils/GUIUtils.h"

// Asset
#include "GEngine/Asset/Mesh.h"

// Editor only
#include "GEngine/Components/Camera/EditorCamera.h"


// Physics
#include "GEngine/Physics/2D/Physics2D.h"
#include "GEngine/Physics/2D/PhysicalContactListener2D.h"

#include "GEngine/Physics/3D/Physics3D.h"
