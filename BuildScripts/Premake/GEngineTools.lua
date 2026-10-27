project "GEngineTools"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"
    targetdir ("%{wks.location}/dist/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/build/" .. outputdir .. "/%{prj.name}")
    files { "../Tools/GEngineTools.cpp" }
    links { "bcrypt" }
    defines { "UNICODE", "_UNICODE" }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release or Dist"
        runtime "Release"
        optimize "On"

    filter {}
