project "yaml-cpp"
	kind "StaticLib"
	language "C++"
	staticruntime "off"

	targetdir ("%{wks.location}/dist/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/build/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{wks.location}/GEngine/GEngine-ThirdParty/yaml-cpp/src/**.h",
		"%{wks.location}/GEngine/GEngine-ThirdParty/yaml-cpp/src/**.cpp",
		"%{wks.location}/GEngine/GEngine-ThirdParty/yaml-cpp/include/**.h",
	}

	includedirs
	{
		"%{wks.location}/GEngine/GEngine-ThirdParty/yaml-cpp/include",
	}

	defines "YAML_CPP_STATIC_DEFINE"

	filter "system:windows"
		systemversion "latest"
		cppdialect "C++17"

	filter "system:linux"
		pic "On"
		systemversion "latest"
		cppdialect "C++17"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
