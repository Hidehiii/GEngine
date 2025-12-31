project "GpuDriven"
	kind "ConsoleApp"
	staticruntime "off"
	cppdialect "C++17"	
	language "C++"
	

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp"
	}

	includedirs
	{
		"%{wks.location}/GEngine/src",
		"%{wks.location}/GEngine/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.GLAD}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.vender}",
		"%{IncludeDir.yaml}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.FBX}"
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
			"GE_PROFILE",
			"FBX_SHARED"
		}

	filter "configurations:Debug"
		defines "GE_DEBUG"
		runtime "Debug"
		symbols "On"

		links
		{
			"%{Library.FBX_SDK_Debug}",
			"%{Library.FBX_XML2_Debug}",
			"%{Library.FBX_Z_Debug}",
		}
	
	filter "configurations:Release"
		defines "GE_RELEASE"
		runtime "Release"
		optimize "On"

		links
		{
			"%{Library.FBX_SDK_Release}",
			"%{Library.FBX_XML2_Release}",
			"%{Library.FBX_Z_Release}",
		}

	filter "configurations:Dist"
		defines "GE_DIST"
		runtime "Release"
		optimize "On"

