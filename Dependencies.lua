

-- GEngine Dep

VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}

IncludeDir["vender"] = "GEngine/vendor"
IncludeDir["GLFW"] = "GEngine/vendor/GLFW/include"
IncludeDir["GLAD"] = "GEngine/vendor/GLAD/include"
IncludeDir["glm"] = "GEngine/vendor/glm"
IncludeDir["stb"] = "GEngine/vendor/stb"
IncludeDir["entt"] = "GEngine/vendor/entt/include"
IncludeDir["ImGui"] = "GEngine/vendor/ImGui"
IncludeDir["yaml"] = "GEngine/vendor/yaml/include"
IncludeDir["ImGuizmo"] = "GEngine/vendor/ImGuizmo"
IncludeDir["SPIRVCross"] = "GEngine/vendor/SPIRVCross"
IncludeDir["shaderc"] = "GEngine/vendor/shaderc/include"
IncludeDir["vulkan"] = "%{VULKAN_SDK}/Include"
IncludeDir["Box2D"] = "GEngine/vendor/box2d/include"


LibraryDir = {}

LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"

Library = {}

Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"

Library["ShaderC_Debug"] = "%{LibraryDir.VulkanSDK}/shaderc_sharedd.lib"
Library["SPIRV_Cross_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] = "%{LibraryDir.VulkanSDK}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["SPIRV_Cross_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"

-- Windows
Library["WinSock"] = "Ws2_32.lib"
Library["WinMM"] = "Winmm.lib"
Library["WinVersion"] = "Version.lib"
Library["BCrypt"] = "Bcrypt.lib"