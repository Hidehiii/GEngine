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

// Graphics
#include "GEngine/Graphics/Graphics.h"
#include "GEngine/Graphics/GraphicsBuffer.h"
#include "GEngine/Graphics/Shader.h"
#include "GEngine/Graphics/GraphicsPipeline.h"
#include "GEngine/Graphics/Texture.h"
#include "GEngine/Graphics/SubTexture2D.h"
#include "GEngine/Graphics/FrameBuffer.h"
#include "GEngine/Graphics/UniformBuffer.h"
#include "GEngine/Graphics/Material.h"
#include "GEngine/Graphics/GraphicsPresent.h"
#include "GEngine/Graphics/Sampler.h"
#include "GEngine/Graphics/RenderPass.h"
#include "GEngine/Graphics/RenderPipeline.h"
#include "GEngine/Graphics/Sampler.h"
#include "GEngine/Graphics/CommandBuffer.h"


// Components
#include "GEngine/Components/Components.h"



// Object
#include "GEngine/Object/Scene/Scene.h"
#include "GEngine/Object/GameObject.h"


// Tools
#include "GEngine/Tools/Serializer.h"
#include "GEngine/Tools/ModelImporter.h"
#include "GEngine/Tools/StringHelper.h"
#include "GEngine/Tools/FileSystemHelper.h"
#include "GEngine/Tools/ShaderCompiler.h"

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

// compute
#include "GEngine/Compute/StorageBuffer.h"
#include "GEngine/Compute/StorageImage.h"
#include "GEngine/Compute/ComputePipeline.h"
