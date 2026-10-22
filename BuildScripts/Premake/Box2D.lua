project "Box2D"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"
    targetdir ("%{wks.location}/dist/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/build/" .. outputdir .. "/%{prj.name}")
    files { "%{wks.location}/GEngine/GEngine-ThirdParty/box2d/src/**.cpp" }
    includedirs { "%{IncludeDir.Box2D}", "%{wks.location}/GEngine/GEngine-ThirdParty/box2d/src" }
    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"
    filter "configurations:Release or Dist"
        runtime "Release"
        optimize "on"
    filter {}
