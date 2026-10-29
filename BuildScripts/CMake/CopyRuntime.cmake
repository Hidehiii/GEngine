if(NOT EXISTS "${GE_ENGINE_DLL}")
    message(FATAL_ERROR "GEngine DLL does not exist: ${GE_ENGINE_DLL}")
endif()
file(COPY_FILE "${GE_ENGINE_DLL}" "${GE_OUTPUT_DIR}/GEngine.dll" ONLY_IF_DIFFERENT)
if(GE_CONFIG STREQUAL "Debug")
    set(physx_config Debug)
else()
    set(physx_config Release)
endif()
file(GLOB runtime_dlls
    "${GE_VENDOR}/PhysX/lib/${physx_config}/*.dll"
    "${GE_VENDOR}/dxc/bin/x64/*.dll"
    "${GE_VENDOR}/OpenSSL/bin/*.dll"
    "${GE_VULKAN_SDK}/Bin/*.dll")
if(NOT runtime_dlls)
    message(FATAL_ERROR "No runtime DLLs found; check GE_VENDOR and VULKAN_SDK.")
endif()
foreach(runtime_dll IN LISTS runtime_dlls)
    get_filename_component(runtime_name "${runtime_dll}" NAME)
    file(COPY_FILE "${runtime_dll}" "${GE_OUTPUT_DIR}/${runtime_name}" ONLY_IF_DIFFERENT)
endforeach()
