set(GE_AGILITY_SDK_ROOT "$ENV{GE_AGILITY_SDK_ROOT}" CACHE PATH "Required prepared Agility SDK runtime root")
if(NOT GE_AGILITY_SDK_ROOT)
    message(FATAL_ERROR "GE_AGILITY_SDK_ROOT is required. Run cmake -P BuildScripts/PrepareAgilitySDK.cmake, then configure -DGE_AGILITY_SDK_ROOT=<output path>.")
endif()
get_filename_component(GE_AGILITY_SDK_ROOT "${GE_AGILITY_SDK_ROOT}" ABSOLUTE)
file(READ "${GE_ROOT}/BuildScripts/AgilitySDK.json" ge_agility_manifest)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${GE_ROOT}/BuildScripts/AgilitySDK.json"
    "${GE_ROOT}/BuildScripts/PrepareAgilitySDK.cmake")
string(JSON GE_AGILITY_SDK_VERSION GET "${ge_agility_manifest}" SDKVersion)
string(JSON GE_AGILITY_VERSION_MS GET "${ge_agility_manifest}" BinaryVersionMS)
string(JSON GE_AGILITY_VERSION_LS GET "${ge_agility_manifest}" BinaryVersionLS)
execute_process(COMMAND "${CMAKE_COMMAND}"
    "-DGE_AGILITY_SDK_ROOT=${GE_AGILITY_SDK_ROOT}" -DGE_AGILITY_CHECK_ONLY=ON
    -P "${GE_ROOT}/BuildScripts/PrepareAgilitySDK.cmake"
    RESULT_VARIABLE ge_agility_result OUTPUT_VARIABLE ge_agility_output ERROR_VARIABLE ge_agility_error)
if(NOT ge_agility_result EQUAL 0)
    message(FATAL_ERROR "Required Agility SDK is invalid: ${ge_agility_error}")
endif()
message(STATUS "${ge_agility_output}")
