cmake_minimum_required(VERSION 3.24)
project(GAssetRegression LANGUAGES CXX)

file(GLOB_RECURSE YAML_CPP_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/../../GEngine/GEngine-ThirdParty/yaml-cpp/src/*.cpp")
add_library(GAssetYaml STATIC ${YAML_CPP_SOURCES})
target_compile_definitions(GAssetYaml PUBLIC YAML_CPP_STATIC_DEFINE)
target_include_directories(GAssetYaml PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/../../GEngine/GEngine-ThirdParty/yaml-cpp/include")

add_executable(GAssetRegression ../GAssetTests.cpp
    "${CMAKE_CURRENT_SOURCE_DIR}/../../GEngine/src/GEngine/Asset/GAsset.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../GEngine/src/GEngine/Core/Log.cpp")
target_compile_features(GAssetRegression PRIVATE cxx_std_17)
target_compile_definitions(GAssetRegression PRIVATE GE_BUILD_DLL)
target_compile_options(GAssetRegression PRIVATE /utf-8)
target_include_directories(GAssetRegression PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../../GEngine/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../GEngine/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../GEngine/GEngine-ThirdParty/spdlog/include")
target_link_libraries(GAssetRegression PRIVATE GAssetYaml)
enable_testing()
add_test(NAME GAssetRegression COMMAND GAssetRegression "${CMAKE_CURRENT_BINARY_DIR}/fixtures"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../Example/FrameGraphTriangle/Assets/ResourceProbe_texture.gasset"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../Example/FrameGraphTriangle/Assets/ResourceProbe_mesh.gasset")
