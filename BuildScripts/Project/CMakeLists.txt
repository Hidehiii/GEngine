cmake_minimum_required(VERSION 3.24)
project(GEngineProject LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
if(NOT IS_DIRECTORY "${GE_PROJECT_ROOT}/src")
    message(FATAL_ERROR "Project source directory src/ is missing")
endif()
set(GE_WITH_MONO OFF CACHE BOOL "Native project build" FORCE)
set(GE_BUILD_QT_EDITOR OFF CACHE BOOL "No editor in game builds" FORCE)
add_subdirectory("${GE_ENGINE_ROOT}/BuildScripts/CMake" "${CMAKE_BINARY_DIR}/engine" EXCLUDE_FROM_ALL)
file(GLOB_RECURSE project_sources CONFIGURE_DEPENDS "${GE_PROJECT_ROOT}/src/*.cpp")
if(NOT project_sources)
    message(FATAL_ERROR "No C++ project sources found in src/")
endif()
add_executable(GProjectGame ${project_sources})
target_include_directories(GProjectGame PRIVATE "${GE_PROJECT_ROOT}/src")
target_compile_definitions(GProjectGame PRIVATE GE_PACKAGED_GAME=1 FBX_SHARED GE_PROFILE UNICODE _UNICODE)
target_compile_options(GProjectGame PRIVATE /utf-8)
target_link_libraries(GProjectGame PRIVATE GEngine ImGui
    "${GE_ENGINE_ROOT}/GEngine/GEngine-ThirdParty/FBXSDK/lib/x64/$<IF:$<CONFIG:Debug>,debug,release>/libfbxsdk-md.lib"
    "${GE_ENGINE_ROOT}/GEngine/GEngine-ThirdParty/FBXSDK/lib/x64/$<IF:$<CONFIG:Debug>,debug,release>/libxml2-md.lib"
    "${GE_ENGINE_ROOT}/GEngine/GEngine-ThirdParty/FBXSDK/lib/x64/$<IF:$<CONFIG:Debug>,debug,release>/zlib-md.lib")
set_target_properties(GProjectGame PROPERTIES OUTPUT_NAME "${GE_PROJECT_NAME}"
    RUNTIME_OUTPUT_DIRECTORY "${GE_DIST_ROOT}/$<CONFIG>/Game/Runtime")
add_executable(GProjectGameLauncher GameLauncher.cpp)
target_compile_definitions(GProjectGameLauncher PRIVATE
    GE_GAME_EXECUTABLE_NAME="${GE_PROJECT_NAME}.exe" UNICODE _UNICODE)
target_compile_options(GProjectGameLauncher PRIVATE /utf-8)
if(GE_PROJECT_ICON)
    file(TO_CMAKE_PATH "${GE_PROJECT_ICON}" GE_PROJECT_ICON_PATH)
    configure_file(GameLauncher.rc.in "${CMAKE_CURRENT_BINARY_DIR}/GameLauncher.rc" @ONLY)
    target_sources(GProjectGameLauncher PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/GameLauncher.rc")
endif()
set_target_properties(GProjectGameLauncher PROPERTIES OUTPUT_NAME "${GE_PROJECT_NAME}"
    RUNTIME_OUTPUT_DIRECTORY "${GE_DIST_ROOT}/$<CONFIG>/Game")
if(DEFINED ENV{VULKAN_SDK} AND NOT "$ENV{VULKAN_SDK}" STREQUAL "")
    set(project_vulkan "$ENV{VULKAN_SDK}")
else()
    set(project_vulkan "${GE_ENGINE_ROOT}/GEngine/GEngine-ThirdParty/VulkanSDK")
endif()
add_custom_target(GProjectPackage DEPENDS GProjectGame GProjectGameLauncher
    COMMAND "${CMAKE_COMMAND}" "-DGE_VENDOR=${GE_ENGINE_ROOT}/GEngine/GEngine-ThirdParty"
        "-DGE_VULKAN_SDK=${project_vulkan}"
        "-DGE_CONFIG=$<CONFIG>" "-DGE_ENGINE_DLL=$<TARGET_FILE:GEngine>"
        "-DGE_OUTPUT_DIR=$<TARGET_FILE_DIR:GProjectGame>"
        -P "${GE_ENGINE_ROOT}/BuildScripts/CMake/CopyRuntime.cmake")
