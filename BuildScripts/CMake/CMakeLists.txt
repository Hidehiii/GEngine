cmake_minimum_required(VERSION 3.24)
project(GEngine LANGUAGES C CXX)

if(NOT WIN32 OR NOT MSVC)
    message(FATAL_ERROR "This CMake build currently supports Windows/MSVC only.")
endif()

set(GE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../..")
get_filename_component(GE_ROOT "${GE_ROOT}" ABSOLUTE)
set(GE_VENDOR "${GE_ROOT}/GEngine/GEngine-ThirdParty")
if(NOT DEFINED ENV{VULKAN_SDK} OR "$ENV{VULKAN_SDK}" STREQUAL "")
    set(GE_VULKAN_SDK "${GE_VENDOR}/VulkanSDK")
else()
    set(GE_VULKAN_SDK "$ENV{VULKAN_SDK}")
endif()
set(GE_FBX_SDK "${GE_VENDOR}/FBXSDK")

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
add_compile_definitions(UNICODE _UNICODE)
add_compile_options(/utf-8)
set(GE_TOOL_ID "cmake" CACHE STRING "Build-tool identifier used in build/dist paths")
set(GE_PLATFORM_ID "windows-x86_64" CACHE STRING "Platform identifier used in build/dist paths")
set(GE_DIST_ROOT "${GE_ROOT}/dist/${GE_TOOL_ID}/${GE_PLATFORM_ID}")

function(ge_set_output_directories target)
    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${GE_DIST_ROOT}/$<CONFIG>/${target}"
        LIBRARY_OUTPUT_DIRECTORY "${GE_DIST_ROOT}/$<CONFIG>/${target}"
        ARCHIVE_OUTPUT_DIRECTORY "${GE_DIST_ROOT}/$<CONFIG>/${target}"
        PDB_OUTPUT_DIRECTORY "${GE_DIST_ROOT}/$<CONFIG>/${target}"
        COMPILE_PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/${target}"
    )
endfunction()

set(GE_INCLUDE_DIRS
    "${GE_ROOT}/GEngine/include"
    "${GE_VENDOR}"
    "${GE_VENDOR}/spdlog/include"
    "${GE_VENDOR}/glfw/include"
    "${GE_VENDOR}/GLAD/include"
    "${GE_VENDOR}/glm"
    "${GE_VENDOR}/stb"
    "${GE_VENDOR}/entt/single_include/entt"
    "${GE_VENDOR}/imgui"
    "${GE_VENDOR}/yaml-cpp/include"
    "${GE_VENDOR}/SPIRV-Cross"
    "${GE_VENDOR}/shaderc/include"
    "${GE_VULKAN_SDK}/Include"
    "${GE_VENDOR}/box2d/include"
    "${GE_FBX_SDK}/include"
    "${GE_VENDOR}/mono/include"
    "${GE_VENDOR}/PhysX/include"
    "${GE_VENDOR}/DirectX-Headers/include"
    "${GE_VENDOR}/HLSLcc/include"
    "${GE_VENDOR}/dxc/inc"
    "${GE_VENDOR}/OpenSSL/include"
)

add_library(GLFW STATIC
    "${GE_VENDOR}/glfw/src/context.c"
    "${GE_VENDOR}/glfw/src/init.c"
    "${GE_VENDOR}/glfw/src/input.c"
    "${GE_VENDOR}/glfw/src/monitor.c"
    "${GE_VENDOR}/glfw/src/null_init.c"
    "${GE_VENDOR}/glfw/src/null_joystick.c"
    "${GE_VENDOR}/glfw/src/null_monitor.c"
    "${GE_VENDOR}/glfw/src/null_window.c"
    "${GE_VENDOR}/glfw/src/platform.c"
    "${GE_VENDOR}/glfw/src/vulkan.c"
    "${GE_VENDOR}/glfw/src/window.c"
    "${GE_VENDOR}/glfw/src/win32_init.c"
    "${GE_VENDOR}/glfw/src/win32_joystick.c"
    "${GE_VENDOR}/glfw/src/win32_module.c"
    "${GE_VENDOR}/glfw/src/win32_monitor.c"
    "${GE_VENDOR}/glfw/src/win32_time.c"
    "${GE_VENDOR}/glfw/src/win32_thread.c"
    "${GE_VENDOR}/glfw/src/win32_window.c"
    "${GE_VENDOR}/glfw/src/wgl_context.c"
    "${GE_VENDOR}/glfw/src/egl_context.c"
    "${GE_VENDOR}/glfw/src/osmesa_context.c"
)
ge_set_output_directories(GLFW)
target_include_directories(GLFW PRIVATE "${GE_VENDOR}/glfw/include" "${GE_VENDOR}/glfw/src")
target_compile_definitions(GLFW PRIVATE _GLFW_WIN32 _CRT_SECURE_NO_WARNINGS GLFW_EXPOSE_NATIVE_WIN32)

add_library(GLAD STATIC "${GE_VENDOR}/GLAD/src/glad.c")
ge_set_output_directories(GLAD)
target_include_directories(GLAD PUBLIC "${GE_VENDOR}/GLAD/include")

add_library(ImGui STATIC
    "${GE_VENDOR}/imgui/imgui.cpp"
    "${GE_VENDOR}/imgui/imgui_draw.cpp"
    "${GE_VENDOR}/imgui/imgui_tables.cpp"
    "${GE_VENDOR}/imgui/imgui_widgets.cpp"
    "${GE_VENDOR}/imgui/backends/imgui_impl_glfw.cpp"
    "${GE_VENDOR}/imgui/backends/imgui_impl_win32.cpp"
    "${GE_VENDOR}/imgui/backends/imgui_impl_opengl3.cpp"
    "${GE_VENDOR}/imgui/backends/imgui_impl_vulkan.cpp"
    "${GE_VENDOR}/imgui/backends/imgui_impl_dx12.cpp"
)
ge_set_output_directories(ImGui)
target_include_directories(ImGui PRIVATE "${GE_VENDOR}/imgui" "${GE_VENDOR}/glfw/include" "${GE_VULKAN_SDK}/Include")

file(GLOB_RECURSE GE_BOX2D_SOURCES CONFIGURE_DEPENDS "${GE_VENDOR}/box2d/src/*.cpp")
add_library(Box2D STATIC ${GE_BOX2D_SOURCES})
ge_set_output_directories(Box2D)
target_include_directories(Box2D PRIVATE "${GE_VENDOR}/box2d/include" "${GE_VENDOR}/box2d/src")

file(GLOB_RECURSE GE_YAML_SOURCES CONFIGURE_DEPENDS "${GE_VENDOR}/yaml-cpp/src/*.cpp")
add_library(yaml-cpp STATIC ${GE_YAML_SOURCES})
ge_set_output_directories(yaml-cpp)
target_include_directories(yaml-cpp PUBLIC "${GE_VENDOR}/yaml-cpp/include")
target_compile_definitions(yaml-cpp PUBLIC YAML_CPP_STATIC_DEFINE)

file(GLOB_RECURSE GE_ENGINE_SOURCES CONFIGURE_DEPENDS "${GE_ROOT}/GEngine/src/*.cpp")
add_library(spirv-cross-core STATIC
    "${GE_VENDOR}/SPIRV-Cross/spirv_cross.cpp"
    "${GE_VENDOR}/SPIRV-Cross/spirv_parser.cpp"
    "${GE_VENDOR}/SPIRV-Cross/spirv_cross_parsed_ir.cpp"
    "${GE_VENDOR}/SPIRV-Cross/spirv_cfg.cpp")
add_library(spirv-cross-glsl STATIC "${GE_VENDOR}/SPIRV-Cross/spirv_glsl.cpp")
foreach(target spirv-cross-core spirv-cross-glsl)
    ge_set_output_directories(${target})
    target_include_directories(${target} PUBLIC "${GE_VENDOR}/SPIRV-Cross")
endforeach()
target_link_libraries(spirv-cross-glsl PUBLIC spirv-cross-core)
add_library(GEngine SHARED ${GE_ENGINE_SOURCES})
ge_set_output_directories(GEngine)
target_include_directories(GEngine PUBLIC ${GE_INCLUDE_DIRS} PRIVATE "${GE_ROOT}/GEngine/src")
target_compile_definitions(GEngine PRIVATE GE_BUILD_DLL _CRT_SECURE_NO_WARNINGS GLFW_INCLUDE_NONE FBX_SHARED GE_PROFILE)
target_compile_definitions(GEngine PUBLIC
    "$<$<CONFIG:Debug>:GE_DEBUG>"
    "$<$<CONFIG:Release>:GE_RELEASE>"
    "$<$<CONFIG:RelWithDebInfo>:GE_RELEASE>"
    "$<$<CONFIG:MinSizeRel>:GE_RELEASE>"
)
target_compile_options(GEngine PRIVATE "$<$<CONFIG:Debug>:/FS>")
target_precompile_headers(GEngine PRIVATE "${GE_ROOT}/GEngine/src/GEpch.h")
target_link_libraries(GEngine PRIVATE GLFW GLAD ImGui yaml-cpp Box2D spirv-cross-core spirv-cross-glsl
    opengl32 d3dcompiler d3d12 dxgi dxguid Ws2_32 Winmm Version Bcrypt
    "${GE_VENDOR}/mono/lib/$<IF:$<CONFIG:Debug>,Debug,Release>/libmono-static-sgen.lib"
    "${GE_VULKAN_SDK}/Lib/vulkan-1.lib"
    "${GE_VENDOR}/PhysX/lib/$<IF:$<CONFIG:Debug>,Debug,Release>/PhysX_64.lib"
    "${GE_VENDOR}/PhysX/lib/$<IF:$<CONFIG:Debug>,Debug,Release>/PhysXCommon_64.lib"
    "${GE_VENDOR}/PhysX/lib/$<IF:$<CONFIG:Debug>,Debug,Release>/PhysXFoundation_64.lib"
    "${GE_VENDOR}/PhysX/lib/$<IF:$<CONFIG:Debug>,Debug,Release>/PhysXCooking_64.lib"
    "${GE_VENDOR}/dxc/lib/x64/dxcompiler.lib"
    "${GE_VENDOR}/dxc/lib/x64/dxil.lib"
    "${GE_VULKAN_SDK}/Lib/$<IF:$<CONFIG:Debug>,shaderc_sharedd.lib,shaderc_shared.lib>"
    "${GE_FBX_SDK}/lib/x64/$<IF:$<CONFIG:Debug>,debug,release>/libfbxsdk-md.lib"
    "${GE_FBX_SDK}/lib/x64/$<IF:$<CONFIG:Debug>,debug,release>/libxml2-md.lib"
    "${GE_FBX_SDK}/lib/x64/$<IF:$<CONFIG:Debug>,debug,release>/$<IF:$<CONFIG:Debug>,zlib-md.lib,zlib-md.lib>"
    "${GE_VENDOR}/OpenSSL/lib/VC/x64/$<IF:$<CONFIG:Debug>,MDd,MD>/libssl.lib"
    "${GE_VENDOR}/OpenSSL/lib/VC/x64/$<IF:$<CONFIG:Debug>,MDd,MD>/libcrypto.lib"
)

function(ge_add_application name directory)
    file(GLOB_RECURSE app_sources CONFIGURE_DEPENDS "${GE_ROOT}/${directory}/src/*.cpp")
    add_executable(${name} ${app_sources})
    ge_set_output_directories(${name})
    target_include_directories(${name} PRIVATE ${GE_INCLUDE_DIRS} "${GE_ROOT}/${directory}/src")
    target_compile_definitions(${name} PRIVATE GE_PROFILE FBX_SHARED)
    target_link_libraries(${name} PRIVATE GEngine ImGui
        "${GE_FBX_SDK}/lib/x64/$<IF:$<CONFIG:Debug>,debug,release>/libfbxsdk-md.lib"
        "${GE_FBX_SDK}/lib/x64/$<IF:$<CONFIG:Debug>,debug,release>/libxml2-md.lib"
        "${GE_FBX_SDK}/lib/x64/$<IF:$<CONFIG:Debug>,debug,release>/zlib-md.lib")
    set_target_properties(${name} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${GE_ROOT}/${directory}")
    add_custom_command(TARGET ${name} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" "-DGE_VENDOR=${GE_VENDOR}"
            "-DGE_VULKAN_SDK=${GE_VULKAN_SDK}" "-DGE_CONFIG=$<CONFIG>"
            "-DGE_OUTPUT_DIR=$<TARGET_FILE_DIR:${name}>"
            "-DGE_ENGINE_DLL=$<TARGET_FILE:GEngine>"
            -P "${CMAKE_CURRENT_LIST_DIR}/CopyRuntime.cmake"
        VERBATIM)
endfunction()

ge_add_application(Sandbox Sandbox)
ge_add_application(GEngine-Editor GEngine-Editor)
target_sources(GEngine-Editor PRIVATE "${GE_VENDOR}/ImGuizmo/src/ImGuizmo.cpp")
target_include_directories(GEngine-Editor PRIVATE "${GE_VENDOR}/ImGuizmo/src")
foreach(example DeferredRender GpuDriven ShaderCompile InputSystem Triangle FrameGraphTriangle SceneRenderHelpers)
    ge_add_application(${example} "Example/${example}")
endforeach()
