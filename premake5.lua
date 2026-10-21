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

filter { "language:C++", "configurations:Release or Dist" }
	defines "NDEBUG"
filter {}

-- Include directories relative to root folder (solution directory)
include "Dependencies.lua"



group "Dependencies"
	include "GEngine/Third-Party/box2d"
	include "GEngine/Third-Party/GLFW"
	include "GEngine/Third-Party/GLAD"
	include "GEngine/Third-Party/ImGui"
	include "GEngine/Third-Party/yaml"
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
