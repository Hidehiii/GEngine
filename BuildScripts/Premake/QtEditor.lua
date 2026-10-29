local sdkRoot = path.getabsolute(_OPTIONS["qt6-sdk"])
local editorRoot = path.join(path.getabsolute("../..", _SCRIPT_DIR), "GEngine-Editor")

local function editorProject(name)
    project(name)
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++17"
        staticruntime "off"
        targetdir ("%{wks.location}/dist/" .. outputdir .. "/%{prj.name}")
        objdir ("%{wks.location}/build/" .. outputdir .. "/%{prj.name}")
        includedirs { editorRoot }
        filter "configurations:Debug"
            runtime "Debug"
            symbols "On"
        filter "configurations:Release or Dist"
            runtime "Release"
            optimize "On"
        filter {}
end

editorProject("GEngine-EditorDiagnosticWorker")
    files { editorRoot .. "/Worker/DiagnosticWorker.cpp", editorRoot .. "/Protocol/**.h" }

editorProject("GEngine-EditorProtocolTests")
    files { editorRoot .. "/Tests/ProtocolTests.cpp", editorRoot .. "/Protocol/**.h", editorRoot .. "/Core/**.h" }

editorProject("GEngine-EditorEngineWorker")
    files { editorRoot .. "/Worker/EngineWorker.cpp", editorRoot .. "/Protocol/**.h" }
    includedirs { editorRoot .. "/../GEngine/include", "%{IncludeDir.spdlog}",
        "%{IncludeDir.glm}", "%{IncludeDir.entt}", "%{IncludeDir.ImGui}",
        "%{IncludeDir.yaml}", "%{IncludeDir.FBX}", "%{IncludeDir.mono}",
        "%{IncludeDir.PhysX}", "%{IncludeDir.vender}" }
    links { "GEngine", "ImGui", "yaml-cpp" }
    defines { "YAML_CPP_STATIC_DEFINE" }
    defines { "FBX_SHARED" }
    filter "configurations:Debug"
        defines { "GE_DEBUG" }
        links { "%{Library.FBX_SDK_Debug}", "%{Library.FBX_XML2_Debug}", "%{Library.FBX_Z_Debug}" }
    filter "configurations:Release or Dist"
        defines { "GE_RELEASE" }
        links { "%{Library.FBX_SDK_Release}", "%{Library.FBX_XML2_Release}", "%{Library.FBX_Z_Release}" }
    filter {}
    postbuildcommands {
        '{COPYFILE} "%{cfg.targetdir}/../GEngine/GEngine.dll" "%{cfg.targetdir}"',
        '{COPYDIR} "' .. editorRoot .. '/Worker/Assets" "%{cfg.targetdir}/Assets"',
        'if not exist "%{cfg.targetdir}/Config.ini" copy /Y "' .. editorRoot .. '/Worker/Config.ini" "%{cfg.targetdir}/Config.ini"',
        '{COPY} "' .. editorRoot .. '/../GEngine/GEngine-ThirdParty/PhysX/lib/%{cfg.buildcfg}/*.dll" "%{cfg.targetdir}"',
        '{COPY} "' .. editorRoot .. '/../GEngine/GEngine-ThirdParty/dxc/bin/x64/*.dll" "%{cfg.targetdir}"',
        '{COPY} "' .. editorRoot .. '/../GEngine/GEngine-ThirdParty/OpenSSL/bin/*.dll" "%{cfg.targetdir}"',
        '{COPY} "%{LibraryDir.VulkanSDK_Bin}*.dll" "%{cfg.targetdir}"'
    }

editorProject("GEngine-EditorHost")
    buildoptions { "/Zc:__cplusplus", "/permissive-" }
    files { editorRoot .. "/Host/**.cpp", editorRoot .. "/Host/**.h",
        editorRoot .. "/Protocol/**.h", editorRoot .. "/Core/**.h" }
    defines { "UNICODE", "_UNICODE", "QT_CORE_LIB", "QT_GUI_LIB", "QT_WIDGETS_LIB" }
    dependson { "GEngine-EditorDiagnosticWorker", "GEngine-EditorEngineWorker" }
    filter "configurations:Debug"
        includedirs { sdkRoot .. "/Debug/include", sdkRoot .. "/Debug/include/QtCore",
            sdkRoot .. "/Debug/include/QtGui", sdkRoot .. "/Debug/include/QtWidgets" }
        libdirs { sdkRoot .. "/Debug/lib" }
        links { "Qt6Cored", "Qt6Guid", "Qt6Widgetsd" }
        postbuildcommands { '"' .. sdkRoot .. '/Debug/bin/windeployqt.exe" --debug --no-translations --no-compiler-runtime "%{cfg.buildtarget.abspath}"' }
        postbuildcommands { '{COPYFILE} "' .. sdkRoot .. '/Debug/plugins/platforms/qoffscreend.dll" "%{cfg.targetdir}/platforms/qoffscreend.dll"' }
    filter "configurations:Release or Dist"
        defines { "QT_NO_DEBUG" }
        includedirs { sdkRoot .. "/Release/include", sdkRoot .. "/Release/include/QtCore",
            sdkRoot .. "/Release/include/QtGui", sdkRoot .. "/Release/include/QtWidgets" }
        libdirs { sdkRoot .. "/Release/lib" }
        links { "Qt6Core", "Qt6Gui", "Qt6Widgets" }
        postbuildcommands { '"' .. sdkRoot .. '/Release/bin/windeployqt.exe" --release --no-translations --no-compiler-runtime "%{cfg.buildtarget.abspath}"' }
        postbuildcommands { '{COPYFILE} "' .. sdkRoot .. '/Release/plugins/platforms/qoffscreen.dll" "%{cfg.targetdir}/platforms/qoffscreen.dll"' }
    filter {}
    postbuildcommands {
        '{COPYFILE} "%{cfg.targetdir}/../GEngineTools/GEngineTools.exe" "%{cfg.targetdir}/GEngineTools.exe"',
        '{MKDIR} "%{cfg.targetdir}/fonts"',
        '{COPYFILE} "' .. editorRoot .. '/Assets/Fonts/Noto_Sans_SC/static/NotoSansSC-Regular.gasset" "%{cfg.targetdir}/fonts/NotoSansSC-Regular.gasset"',
        '{COPYFILE} "' .. editorRoot .. '/Assets/Fonts/Noto_Sans_SC/OFL.txt" "%{cfg.targetdir}/fonts/OFL.txt"'
    }
