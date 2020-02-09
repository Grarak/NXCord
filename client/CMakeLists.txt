cmake_minimum_required(VERSION 3.1)

project(client)

option(ENABLE_CLIENT_STANDALONE "Enable standalone client without IPC with sysmodule" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB client_src
        ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/ui/*.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/ui/elements/*.cpp)

add_executable(client ${client_src})

if (ENABLE_CLIENT_STANDALONE)
    target_compile_definitions(client PRIVATE STANDALONE)
endif ()

string(TIMESTAMP TODAY "%Y%m%d")
set_target_properties(client PROPERTIES
        APP_TITLE "NXCord"
        APP_AUTHOR "Grarak"
        APP_VERSION "0.2-${TODAY}")

target_link_libraries(client plutonium nxcord)

add_nro_target(client)