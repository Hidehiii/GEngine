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


group "Core"
	include "GEngine-ScriptCore"
	include "GEngine"
group ""

group "Tools"
	include "GEngine-Editor"
group ""

include "Sandbox"
