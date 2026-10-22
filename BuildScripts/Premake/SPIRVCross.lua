for _, component in ipairs { "core", "glsl" } do
    project ("spirv-cross-" .. component)
        kind "StaticLib"
        language "C++"
        cppdialect "C++17"
        staticruntime "off"
        targetdir ("%{wks.location}/dist/" .. outputdir .. "/%{prj.name}")
        objdir ("%{wks.location}/build/" .. outputdir .. "/%{prj.name}")
        includedirs { "%{IncludeDir.SPIRVCross}" }
        if component == "core" then
            for _, source in ipairs { "spirv_cross", "spirv_parser", "spirv_cross_parsed_ir", "spirv_cfg" } do
                files { "%{wks.location}/GEngine/GEngine-ThirdParty/SPIRV-Cross/" .. source .. ".cpp" }
            end
        else
            files { "%{wks.location}/GEngine/GEngine-ThirdParty/SPIRV-Cross/spirv_glsl.cpp" }
            links { "spirv-cross-core" }
        end
        filter "configurations:Debug"
            runtime "Debug"
            symbols "on"
        filter "configurations:Release or Dist"
            runtime "Release"
            optimize "on"
        filter {}
end
