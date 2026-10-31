local backendIncludeDirs =
{
	"include",
	"src",
	"%{IncludeDir.spdlog}",
	"%{IncludeDir.GLFW}",
	"%{IncludeDir.GLAD}",
	"%{IncludeDir.glm}",
	"%{IncludeDir.stb}",
	"%{IncludeDir.entt}",
	"%{IncludeDir.vender}",
	"%{IncludeDir.ImGui}",
	"%{IncludeDir.yaml}",
	"%{IncludeDir.SPIRVCross}",
	"%{IncludeDir.shaderc}",
	"%{IncludeDir.vulkan}",
	"%{IncludeDir.Box2D}",
	"%{IncludeDir.PhysX}",
	"%{IncludeDir.DirectXHeaders}",
	"%{IncludeDir.HLSLcc}",
	"%{IncludeDir.dxc}",
}

local function graphicsBackendProject(name, sourcePath, headerPath)
	project(name)
	kind "StaticLib"
	staticruntime "off"
	language "C++"

	targetdir ("%{wks.location}/dist/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/build/" .. outputdir .. "/%{prj.name}")

	files
	{
		sourcePath,
		headerPath,
	}

	includedirs(backendIncludeDirs)

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE",
		"FBX_SHARED",
		"YAML_CPP_STATIC_DEFINE",
		"GE_BUILD_DLL",
		"GE_PROFILE",
	}

	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"

	filter "configurations:Debug"
		defines "GE_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release or Dist"
		defines "GE_RELEASE"
		runtime "Release"
		optimize "On"

	filter {}
end

local function physicsBackendProject(name, sourcePath, headerPath)
	project(name)
	kind "StaticLib"
	staticruntime "off"
	language "C++"

	targetdir ("%{wks.location}/dist/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/build/" .. outputdir .. "/%{prj.name}")

	files
	{
		sourcePath,
		headerPath,
	}

	includedirs(backendIncludeDirs)

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE",
		"FBX_SHARED",
		"YAML_CPP_STATIC_DEFINE",
		"GE_BUILD_DLL",
		"GE_PROFILE",
	}

	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"

	filter "configurations:Debug"
		defines "GE_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release or Dist"
		defines "GE_RELEASE"
		runtime "Release"
		optimize "On"

	filter {}
end

graphicsBackendProject("GEngine-OpenGLBackend", "src/Platform/OpenGL/**.cpp",
	"include/Platform/OpenGL/**.h")
links
{
	"GLFW",
	"GLAD",
	"ImGui",
	"spirv-cross-core",
	"spirv-cross-glsl",
	"opengl32.lib",
}

graphicsBackendProject("GEngine-VulkanBackend", "src/Platform/Vulkan/**.cpp",
	"include/Platform/Vulkan/**.h")
links
{
	"GLFW",
	"ImGui",
	"spirv-cross-core",
	"%{Library.Vulkan}",
}

graphicsBackendProject("GEngine-D3D12Backend", "src/Platform/D3D12/**.cpp",
	"include/Platform/D3D12/**.h")
links
{
	"GLFW",
	"ImGui",
	"d3dcompiler.lib",
	"d3d12.lib",
	"dxgi.lib",
	"dxguid.lib",
	"%{Library.dxc_dxcompiler}",
	"%{Library.dxc_dxil}",
}

physicsBackendProject("GEngine-Physics2DBackend", "src/GEngine/Physics/2D/**.cpp",
	"include/GEngine/Physics/2D/**.h")
links
{
	"Box2D",
}

physicsBackendProject("GEngine-Physics3DBackend", "src/GEngine/Physics/3D/**.cpp",
	"include/GEngine/Physics/3D/**.h")
links
{
	"%{Library.PhysX_64}",
	"%{Library.PhysXCommon_64}",
	"%{Library.PhysXFoundation_64}",
	"%{Library.PhysXCooking_64}",
}

project "GEngine"
	kind "SharedLib"
	staticruntime "off"
	--cppdialect "C++20"	
	language "C++"
	

	targetdir ("%{wks.location}/dist/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/build/" .. outputdir .. "/%{prj.name}")

	pchheader "GEpch.h"
	pchsource "src/GEpch.cpp"

	filter "files:src/Platform/Windows/WindowsResourcePack.cpp"
		enablepch "Off"
	filter {}

	files
	{
		"include/**.h",
		"src/**.cpp",
		"GEngine-ThirdParty/glm/glm/**.hpp",
		"GEngine-ThirdParty/glm/glm/**.inl",
		"GEngine-ThirdParty/entt/single_include/entt/**.hpp",
		
	}

	removefiles
	{
		"src/Platform/OpenGL/**.cpp",
		"src/Platform/Vulkan/**.cpp",
		"src/Platform/D3D12/**.cpp",
		"src/GEngine/Physics/2D/**.cpp",
		"src/GEngine/Physics/3D/**.cpp",
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE",
		"FBX_SHARED",
		"YAML_CPP_STATIC_DEFINE"
	}


	includedirs
	{
		"include",
		"src",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.GLAD}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.vender}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.yaml}",
		"%{IncludeDir.SPIRVCross}",
		"%{IncludeDir.shaderc}",
		"%{IncludeDir.vulkan}",
		"%{IncludeDir.Box2D}",
		"%{IncludeDir.FBX}",
		"%{IncludeDir.mono}",
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.DirectXHeaders}",
		"%{IncludeDir.HLSLcc}",
		"%{IncludeDir.dxc}",
		"%{IncludeDir.OpenSSL}",
	}

	links
	{
		"GEngine-OpenGLBackend",
		"GEngine-VulkanBackend",
		"GEngine-D3D12Backend",
		"GEngine-Physics2DBackend",
		"GEngine-Physics3DBackend",
		"GLFW",
		"GLAD",
		"ImGui",
		"yaml-cpp",
	}

	if _OPTIONS["without-mono"] then
		defines { "GE_WITH_MONO=0" }
	else
		links { "%{Library.mono}" }
	end

	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"

		defines
		{
			"GE_BUILD_DLL",
			"GE_PROFILE"
		}

		links
		{
			"%{Library.WinSock}",
			"%{Library.WinMM}",
			"%{Library.WinVersion}",
			"%{Library.BCrypt}",

			"%{Library.dxc_dxcompiler}",
			"%{Library.dxc_dxil}"
		}

		postbuildcommands
		{
			("for /d %%d in (\"%{wks.location}/dist/" .. outputdir .. "/*\") do IF \"%%d\\\" neq \"%{cfg.buildtarget.directory}\" (xcopy /Q /Y /I \"%{cfg.buildtarget.directory}%{cfg.buildtarget.name}\" \"%%d\\\" > nul) ELSE (echo Skipping copy \"%{cfg.buildtarget.directory}%{cfg.buildtarget.name}\" to \"%%d\")"),
			("for /d %%d in (\"%{wks.location}/dist/" .. outputdir .. "/*\") do {COPY} \"%{LibraryDir.PhysX}*.dll\" \"%%d\\\""),
			("for /d %%d in (\"%{wks.location}/dist/" .. outputdir .. "/*\") do {COPY} \"%{LibraryDir.VulkanSDK_Bin}*.dll\" \"%%d\\\""),
			("for /d %%d in (\"%{wks.location}/dist/" .. outputdir .. "/*\") do {COPY} \"%{LibraryDir.dxc_dll}*.dll\" \"%%d\\\""),
			("for /d %%d in (\"%{wks.location}/dist/" .. outputdir .. "/*\") do {COPY} \"%{LibraryDir.OpenSSL_dll}*.dll\" \"%%d\\\""),
		}

	filter "configurations:Debug"
		defines "GE_DEBUG"
		-- Debug builds compile multiple translation units in parallel. /FS serializes
		-- shared PDB writes so they do not intermittently fail with C1041.
		buildoptions { "/FS" }
		runtime "Debug"
		symbols "On"

		links
		{
			"%{Library.ShaderC_Debug}",
			"%{Library.SPIRV_Cross_Debug}",
			"%{Library.SPIRV_Cross_GLSL_Debug}",

			"%{Library.FBX_SDK_Debug}",
			"%{Library.FBX_XML2_Debug}",
			"%{Library.FBX_Z_Debug}",

			"%{Library.OpenSSL_ssl_Debug}",
			"%{Library.OpenSSL_crypto_Debug}",
		}
	
	filter "configurations:Release"
		defines "GE_RELEASE"
		runtime "Release"
		optimize "On"

		links
		{
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}",

			"%{Library.FBX_SDK_Release}",
			"%{Library.FBX_XML2_Release}",
			"%{Library.FBX_Z_Release}",

			"%{Library.OpenSSL_ssl_Release}",
			"%{Library.OpenSSL_crypto_Release}",
		}

	filter "configurations:Dist"
		defines "GE_DIST"
		runtime "Release"
		optimize "On"

		links
		{
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}",

			"%{Library.FBX_SDK_Release}",
			"%{Library.FBX_XML2_Release}",
			"%{Library.FBX_Z_Release}",

			"%{Library.OpenSSL_ssl_Release}",
			"%{Library.OpenSSL_crypto_Release}",
		}
