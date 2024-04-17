workspace "GEngine"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	startproject "GEngine-Editor"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
include "Dependencies.lua"


group "Dependencies"
	include "GEngine/vendor/Box2D"
	include "GEngine/vendor/GLFW"
	include "GEngine/vendor/GLAD"
	include "GEngine/vendor/ImGui"
	include "GEngine/vendor/yaml"
group ""



project "GEngine"
	location "GEngine"
	kind "SharedLib"
	staticruntime "off"
	cppdialect "C++17"	
	language "C++"
	

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "GEpch.h"
	pchsource "GEngine/src/GEpch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/glm/**.hpp",
		"%{prj.name}/vendor/glm/**.inl",
		"%{prj.name}/vendor/entt/include/**.hpp",
		"%{prj.name}/vendor/ImGuizmo/ImGuizmo.h",
		"%{prj.name}/vendor/ImGuizmo/ImGuizmo.cpp",
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE"
	}


	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.GLAD}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.vender}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.yaml}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.SPIRVCross}",
		"%{IncludeDir.shaderc}",
		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.vulkan}",
		"%{IncludeDir.Box2D}",
	}

	links
	{
		"GLFW",
		"GLAD",
		"ImGui",
		"yaml-cpp",
		"Box2D",
		"opengl32.lib",
	}

	filter "files:GEngine/vendor/ImGuizmo/**.cpp"
		flags
		{
			"NoPCH"
		}

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
		}

		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} \"../bin/" .. outputdir .. "/Sandbox/\""),
			("{COPY} %{cfg.buildtarget.relpath} \"../bin/" .. outputdir .. "/GEngine-Editor/\"")
		}

	filter "configurations:Debug"
		defines "GE_DEBUG"
		runtime "Debug"
		symbols "On"

		links
		{
			"%{Library.ShaderC_Debug}",
			"%{Library.SPIRV_Cross_Debug}",
			"%{Library.SPIRV_Cross_GLSL_Debug}"
		}
	
	filter "configurations:Release"
		defines "GE_RELEASE"
		runtime "Release"
		optimize "On"

		links
		{
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}

	filter "configurations:Dist"
		defines "GE_DIST"
		runtime "Release"
		optimize "On"

		links
		{
			"%{Library.ShaderC_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}"
		}

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	staticruntime "off"
	cppdialect "C++17"	
	language "C++"
	

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"GEngine/src",
		"GEngine/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.GLAD}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.vender}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.yaml}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.SPIRVCross}",
		"%{IncludeDir.shaderc}",
	}

	links
	{
		"GEngine"
	}

	filter "system:windows"
		cppdialect "C++17"	
		systemversion "latest"

		defines
		{
			"GE_PROFILE"
		}

	filter "configurations:Debug"
		defines "GE_DEBUG"
		runtime "Debug"
		symbols "On"
	
	filter "configurations:Release"
		defines "GE_RELEASE"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		defines "GE_DIST"
		runtime "Release"
		optimize "On"

project "GEngine-Editor"
	location "GEngine-Editor"
	kind "ConsoleApp"
	staticruntime "off"
	language "C++"
	

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"GEngine/src",
		"GEngine/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.GLAD}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.vender}",
		"%{IncludeDir.yaml}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGuizmo}",

	}

	links
	{
		"GEngine",
		"ImGui"
	}

	filter "system:windows"
		cppdialect "C++17"	
		systemversion "latest"

		defines
		{
			"GE_PLATFORM_WINDOWS",
			"GE_PROFILE"
		}

	filter "configurations:Debug"
		defines "GE_DEBUG"
		runtime "Debug"
		symbols "On"
	
	filter "configurations:Release"
		defines "GE_RELEASE"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		defines "GE_DIST"
		runtime "Release"
		optimize "On"