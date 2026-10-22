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
	include "GEngine-Editor"
group ""

include "Sandbox"

group "Example"
	include "Example"
group ""
