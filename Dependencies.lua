

-- GEngine Dep

-- VULKAN_SDK = "%{wks.location}/GEngine/vendor/VulkanSDK"
VULKAN_SDK = os.getenv("VULKAN_SDK")
FBX_SDK = "%{wks.location}/GEngine/vendor/FBXSDK"

IncludeDir = {}

IncludeDir["vender"] = "%{wks.location}/GEngine/vendor"
IncludeDir["GLFW"] = "%{wks.location}/GEngine/vendor/GLFW/include"
IncludeDir["GLAD"] = "%{wks.location}/GEngine/vendor/GLAD/include"
IncludeDir["glm"] = "%{wks.location}/GEngine/vendor/glm"
IncludeDir["stb"] = "%{wks.location}/GEngine/vendor/stb"
IncludeDir["entt"] = "%{wks.location}/GEngine/vendor/entt/include"
IncludeDir["ImGui"] = "%{wks.location}/GEngine/vendor/ImGui"
IncludeDir["yaml"] = "%{wks.location}/GEngine/vendor/yaml/include"
IncludeDir["ImGuizmo"] = "%{wks.location}/GEngine/vendor/ImGuizmo"
IncludeDir["SPIRVCross"] = "%{wks.location}/GEngine/vendor/SPIRVCross"
IncludeDir["shaderc"] = "%{wks.location}/GEngine/vendor/shaderc/include"
IncludeDir["vulkan"] = "%{VULKAN_SDK}/Include"
IncludeDir["Box2D"] = "%{wks.location}/GEngine/vendor/box2d/include"
IncludeDir["FBX"] = "%{FBX_SDK}/include"
IncludeDir["mono"] = "%{wks.location}/GEngine/vendor/mono/include"
IncludeDir["PhysX"] = "%{wks.location}/GEngine/vendor/PhysX/include"


LibraryDir = {}

LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibraryDir["VulkanSDK_Bin"] = "%{VULKAN_SDK}/Bin/"
LibraryDir["FBX_SDK"] = "%{FBX_SDK}/lib"
LibraryDir["mono"] = "%{wks.location}/GEngine/vendor/mono/lib/%{cfg.buildcfg}"
LibraryDir["PhysX"] = "%{wks.location}/GEngine/vendor/PhysX/lib/%{cfg.buildcfg}/"

Library = {}

Library["mono"] = "%{LibraryDir.mono}/libmono-static-sgen.lib"

Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"

Library["FBX_SDK_Debug"] = "%{LibraryDir.FBX_SDK}/x64/debug/libfbxsdk-md.lib"
Library["FBX_XML2_Debug"] = "%{LibraryDir.FBX_SDK}/x64/debug/libxml2-md.lib"
Library["FBX_Z_Debug"] = "%{LibraryDir.FBX_SDK}/x64/debug/zlib-md.lib"   

Library["FBX_SDK_Release"] = "%{LibraryDir.FBX_SDK}/x64/release/libfbxsdk-md.lib"
Library["FBX_XML2_Release"] = "%{LibraryDir.FBX_SDK}/x64/release/libxml2-md.lib"
Library["FBX_Z_Release"] = "%{LibraryDir.FBX_SDK}/x64/release/zlib-md.lib"

Library["ShaderC_Debug"] = "%{LibraryDir.VulkanSDK}/shaderc_sharedd.lib"
Library["SPIRV_Cross_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] = "%{LibraryDir.VulkanSDK}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["SPIRV_Cross_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"

Library["PhysX_64"] = "%{LibraryDir.PhysX}PhysX_64.lib"
Library["PhysXCommon_64"] = "%{LibraryDir.PhysX}PhysXCommon_64.lib"
Library["PhysXFoundation_64"] = "%{LibraryDir.PhysX}PhysXFoundation_64.lib"
Library["PhysXCooking_64"] = "%{LibraryDir.PhysX}PhysXCooking_64.lib"
Library["PhysXGPU_64"] = "%{LibraryDir.PhysX}PhysXGpu_64.dll"



-- Windows
Library["WinSock"] = "Ws2_32.lib"
Library["WinMM"] = "Winmm.lib"
Library["WinVersion"] = "Version.lib"
Library["BCrypt"] = "Bcrypt.lib"