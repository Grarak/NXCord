cmake_minimum_required(VERSION 3.1)

project(sysmodule)

option(ENABLE_SYSMODULE_APPLICATION "Enable application sysmodule without IPC with client" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB sysmodule_src ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)

add_executable(sysmodule ${sysmodule_src})

target_link_libraries(sysmodule nxcord stratosphere)

if (ENABLE_SYSMODULE_APPLICATION)
    set_target_properties(sysmodule PROPERTIES
            APP_TITLE "NXCord"
            APP_AUTHOR "Grarak"
            APP_VERSION "Debug")

    target_compile_definitions(sysmodule PRIVATE APPLICATION)
    add_nro_target(sysmodule)
else ()
    set_target_properties(sysmodule PROPERTIES
            CONFIG_JSON "${CMAKE_CURRENT_LIST_DIR}/sysmodule.json")
    add_nsp_target(sysmodule)
endif ()
