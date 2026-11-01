cmake_minimum_required(VERSION 3.24)
project(ShaderContainerChecks LANGUAGES CXX)
if(NOT MSVC OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "These CPU-only checks use the engine's Windows x64 headers.")
endif()
get_filename_component(GE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
add_executable(ShaderContainerChecks
    ../ShaderContainerChecks.cpp
    "${GE_ROOT}/GEngine/src/GEngine/Graphics/ShaderContainer.cpp")
target_compile_features(ShaderContainerChecks PRIVATE cxx_std_17)
target_compile_definitions(ShaderContainerChecks PRIVATE GE_BUILD_DLL "$<$<CONFIG:Debug>:GE_DEBUG>")
target_compile_options(ShaderContainerChecks PRIVATE /utf-8 /W4 /permissive-)
target_include_directories(ShaderContainerChecks PRIVATE
    "${GE_ROOT}/GEngine/include"
    "${GE_ROOT}/GEngine/GEngine-ThirdParty/spdlog/include")
enable_testing()
add_test(NAME ShaderContainerChecks COMMAND ShaderContainerChecks)
