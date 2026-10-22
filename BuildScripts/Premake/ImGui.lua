project "ImGui"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"
    targetdir ("%{wks.location}/dist/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/build/" .. outputdir .. "/%{prj.name}")
    files {
        "%{wks.location}/GEngine/GEngine-ThirdParty/imgui/imgui.cpp",
        "%{wks.location}/GEngine/GEngine-ThirdParty/imgui/imgui_draw.cpp",
        "%{wks.location}/GEngine/GEngine-ThirdParty/imgui/imgui_tables.cpp",
        "%{wks.location}/GEngine/GEngine-ThirdParty/imgui/imgui_widgets.cpp",
        "%{wks.location}/GEngine/GEngine-ThirdParty/imgui/backends/imgui_impl_glfw.cpp",
        "%{wks.location}/GEngine/GEngine-ThirdParty/imgui/backends/imgui_impl_win32.cpp",
        "%{wks.location}/GEngine/GEngine-ThirdParty/imgui/backends/imgui_impl_opengl3.cpp",
        "%{wks.location}/GEngine/GEngine-ThirdParty/imgui/backends/imgui_impl_vulkan.cpp",
        "%{wks.location}/GEngine/GEngine-ThirdParty/imgui/backends/imgui_impl_dx12.cpp"
    }
    includedirs { "%{IncludeDir.ImGui}", "%{IncludeDir.GLFW}", "%{IncludeDir.vulkan}" }
    filter "configurations:Debug"
        runtime "Debug"
        symbols "on"
    filter "configurations:Release or Dist"
        runtime "Release"
        optimize "on"
    filter {}
