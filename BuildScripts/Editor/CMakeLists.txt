cmake_minimum_required(VERSION 3.24)
project(GEngineQtEditor LANGUAGES CXX)
if(NOT WIN32 OR NOT MSVC)
    message(FATAL_ERROR "The initial editor worker transport currently supports Windows/MSVC only.")
endif()
get_filename_component(GE_EDITOR_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
set(GE_EDITOR_SOURCE "${GE_EDITOR_ROOT}/GEngine-Editor")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
set(GE_EDITOR_TOOL_ID "qt-editor" CACHE STRING "Editor output tool identifier")
option(GE_EDITOR_PROTOCOL_ONLY "Build dependency-free protocol tests and diagnostic worker only" OFF)
function(ge_editor_output target)
    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${GE_EDITOR_ROOT}/dist/${GE_EDITOR_TOOL_ID}/windows-x86_64/$<CONFIG>/${target}"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>/${target}")
    target_compile_options(${target} PRIVATE /utf-8 /W4)
    target_include_directories(${target} PRIVATE "${GE_EDITOR_SOURCE}")
endfunction()
add_executable(GEngine-EditorDiagnosticWorker "${GE_EDITOR_SOURCE}/Worker/DiagnosticWorker.cpp")
ge_editor_output(GEngine-EditorDiagnosticWorker)
add_executable(GEngine-EditorProtocolTests "${GE_EDITOR_SOURCE}/Tests/ProtocolTests.cpp")
ge_editor_output(GEngine-EditorProtocolTests)
enable_testing()
add_test(NAME EditorProtocol COMMAND GEngine-EditorProtocolTests)
if(NOT GE_EDITOR_PROTOCOL_ONLY)
    find_package(Qt6 6.10.3 EXACT REQUIRED COMPONENTS Core Gui Widgets)
    add_executable(GEngine-EditorHost
        "${GE_EDITOR_SOURCE}/Host/main.cpp"
        "${GE_EDITOR_SOURCE}/Host/MainWindow.cpp"
        "${GE_EDITOR_SOURCE}/Host/ProjectWorkspace.cpp"
        "${GE_EDITOR_SOURCE}/Host/WorkerSupervisor.cpp"
        "${GE_EDITOR_SOURCE}/Host/RecoveryCheck.cpp")
    ge_editor_output(GEngine-EditorHost)
    target_link_libraries(GEngine-EditorHost PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
    add_dependencies(GEngine-EditorHost GEngine-EditorDiagnosticWorker)
    get_target_property(qt_qmake Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(qt_bin "${qt_qmake}" DIRECTORY)
    find_program(GE_WINDEPLOYQT windeployqt HINTS "${qt_bin}" REQUIRED NO_DEFAULT_PATH)
    add_custom_command(TARGET GEngine-EditorHost POST_BUILD
        COMMAND "${GE_WINDEPLOYQT}" "$<IF:$<CONFIG:Debug>,--debug,--release>"
            --no-translations --no-compiler-runtime "$<TARGET_FILE:GEngine-EditorHost>"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${qt_bin}/../plugins/platforms/$<IF:$<CONFIG:Debug>,qoffscreend.dll,qoffscreen.dll>"
            "$<TARGET_FILE_DIR:GEngine-EditorHost>/platforms/$<IF:$<CONFIG:Debug>,qoffscreend.dll,qoffscreen.dll>"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "$<TARGET_FILE_DIR:GEngine-EditorHost>/fonts"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${GE_EDITOR_SOURCE}/Assets/Fonts/Noto_Sans_SC/static/NotoSansSC-Regular.ttf"
            "$<TARGET_FILE_DIR:GEngine-EditorHost>/fonts/NotoSansSC-Regular.ttf"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${GE_EDITOR_SOURCE}/Assets/Fonts/Noto_Sans_SC/OFL.txt"
            "$<TARGET_FILE_DIR:GEngine-EditorHost>/fonts/OFL.txt"
        VERBATIM)
    add_test(NAME EditorRecovery COMMAND GEngine-EditorHost --self-test)
    set_tests_properties(EditorRecovery PROPERTIES TIMEOUT 35 ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endif()
