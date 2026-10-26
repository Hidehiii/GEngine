

-- GEngine Dep

local vulkanSdkOverride = os.getenv("VULKAN_SDK")
if not vulkanSdkOverride or vulkanSdkOverride == "" then
	vulkanSdkOverride = "%{wks.location}/GEngine/GEngine-ThirdParty/VulkanSDK"
end
VULKAN_SDK = vulkanSdkOverride
FBX_SDK = "%{wks.location}/GEngine/GEngine-ThirdParty/FBXSDK"

IncludeDir = {}

IncludeDir["vender"] = "%{wks.location}/GEngine/GEngine-ThirdParty"
IncludeDir["spdlog"] = "%{wks.location}/GEngine/GEngine-ThirdParty/spdlog/include"
IncludeDir["GLFW"] = "%{wks.location}/GEngine/GEngine-ThirdParty/glfw/include"
IncludeDir["GLAD"] = "%{wks.location}/GEngine/GEngine-ThirdParty/GLAD/include"
IncludeDir["glm"] = "%{wks.location}/GEngine/GEngine-ThirdParty/glm"
IncludeDir["stb"] = "%{wks.location}/GEngine/GEngine-ThirdParty/stb"
IncludeDir["entt"] = "%{wks.location}/GEngine/GEngine-ThirdParty/entt/single_include/entt"
IncludeDir["ImGui"] = "%{wks.location}/GEngine/GEngine-ThirdParty/imgui"
IncludeDir["yaml"] = "%{wks.location}/GEngine/GEngine-ThirdParty/yaml-cpp/include"
IncludeDir["SPIRVCross"] = "%{wks.location}/GEngine/GEngine-ThirdParty/SPIRV-Cross"
IncludeDir["shaderc"] = "%{wks.location}/GEngine/GEngine-ThirdParty/shaderc/include"
IncludeDir["vulkan"] = "%{VULKAN_SDK}/Include"
IncludeDir["Box2D"] = "%{wks.location}/GEngine/GEngine-ThirdParty/box2d/include"
IncludeDir["FBX"] = "%{FBX_SDK}/include"
IncludeDir["mono"] = "%{wks.location}/dist/mono/windows-x86_64/6.10.0.87/Mono/include/mono-2.0"
IncludeDir["PhysX"] = "%{wks.location}/GEngine/GEngine-ThirdParty/PhysX/include"
IncludeDir["DirectXHeaders"] = "%{wks.location}/GEngine/GEngine-ThirdParty/DirectX-Headers/include"
IncludeDir["HLSLcc"] = "%{wks.location}/GEngine/GEngine-ThirdParty/HLSLcc/include"
IncludeDir["dxc"] = "%{wks.location}/GEngine/GEngine-ThirdParty/dxc/inc"
IncludeDir["OpenSSL"] = "%{wks.location}/GEngine/GEngine-ThirdParty/OpenSSL/include"

LibraryDir = {}

LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibraryDir["VulkanSDK_Bin"] = "%{VULKAN_SDK}/Bin/"   
LibraryDir["FBX_SDK"] = "%{FBX_SDK}/lib"
LibraryDir["mono"] = "%{wks.location}/dist/mono/windows-x86_64/6.10.0.87/Mono/lib"
LibraryDir["PhysX"] = "%{wks.location}/GEngine/GEngine-ThirdParty/PhysX/lib/%{cfg.buildcfg}/"
LibraryDir["dxc"] = "%{wks.location}/GEngine/GEngine-ThirdParty/dxc/lib/x64"
LibraryDir["dxc_dll"] = "%{wks.location}/GEngine/GEngine-ThirdParty/dxc/bin/x64/"
LibraryDir["OpenSSL"] = "%{wks.location}/GEngine/GEngine-ThirdParty/OpenSSL/lib/VC/x64"
LibraryDir["OpenSSL_dll"] = "%{wks.location}/GEngine/GEngine-ThirdParty/OpenSSL/bin/"

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
Library["SPIRV_Cross_Debug"] = "spirv-cross-core"
Library["SPIRV_Cross_GLSL_Debug"] = "spirv-cross-glsl"
Library["SPIRV_Tools_Debug"] = "%{LibraryDir.VulkanSDK}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["SPIRV_Cross_Release"] = "spirv-cross-core"
Library["SPIRV_Cross_GLSL_Release"] = "spirv-cross-glsl"

Library["PhysX_64"] = "%{LibraryDir.PhysX}PhysX_64.lib"
Library["PhysXCommon_64"] = "%{LibraryDir.PhysX}PhysXCommon_64.lib"
Library["PhysXFoundation_64"] = "%{LibraryDir.PhysX}PhysXFoundation_64.lib"
Library["PhysXCooking_64"] = "%{LibraryDir.PhysX}PhysXCooking_64.lib"

Library["dxc_dxcompiler"] = "%{LibraryDir.dxc}/dxcompiler.lib"
Library["dxc_dxil"] = "%{LibraryDir.dxc}/dxil.lib"

Library["OpenSSL_ssl_Debug"] = "%{LibraryDir.OpenSSL}/MDd/libssl.lib"
Library["OpenSSL_crypto_Debug"] = "%{LibraryDir.OpenSSL}/MDd/libcrypto.lib"

Library["OpenSSL_ssl_Release"] = "%{LibraryDir.OpenSSL}/MD/libssl.lib"
Library["OpenSSL_crypto_Release"] = "%{LibraryDir.OpenSSL}/MD/libcrypto.lib"

-- Windows
Library["WinSock"] = "Ws2_32.lib"
Library["WinMM"] = "Winmm.lib"
Library["WinVersion"] = "Version.lib"
Library["BCrypt"] = "Bcrypt.lib"
