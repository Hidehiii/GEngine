project "GEngine-Editor"
	kind "ConsoleApp"
	staticruntime "off"
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
		"src",
		"%{wks.location}/GEngine/src",
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