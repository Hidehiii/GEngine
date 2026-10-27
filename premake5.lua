newoption { trigger = "without-mono", description = "Build native-only games without managed scripting support" }
newoption {
    trigger = "qt6-sdk",
    value = "PATH",
    description = "Opt in to Qt editor targets using a root containing Debug/Release Qt SDKs"
}

workspace "GEngine"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	startproject "Sandbox"

outputdir = "premake-%{_ACTION}/%{cfg.system}-%{cfg.architecture}/%{cfg.buildcfg}"

buildoptions { "/utf-8" }

filter { "language:C++", "configurations:Release or Dist" }
	defines "NDEBUG"
filter {}

-- Include directories relative to root folder (solution directory)
include "Dependencies.lua"



group "Dependencies"
	include "BuildScripts/Premake/Box2D.lua"
	include "BuildScripts/Premake/GLFW.lua"
	include "GEngine/GEngine-ThirdParty/GLAD"
	include "BuildScripts/Premake/ImGui.lua"
	include "BuildScripts/Premake/SPIRVCross.lua"
	include "BuildScripts/Premake/YamlCpp.lua"
group ""


group "Core"
	include "GEngine-ScriptCore"
	include "GEngine"
group ""

group "Tools"
    if _OPTIONS["qt6-sdk"] then
        include "BuildScripts/Premake/GEngineTools.lua"
        include "BuildScripts/Premake/QtEditor.lua"
    end
group ""

include "Sandbox"

group "Example"
	include "Example"
group ""
