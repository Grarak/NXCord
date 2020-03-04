cmake_minimum_required(VERSION 3.1)

project(overlay)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB overlay_src ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)

add_executable(overlay ${overlay_src})

target_link_libraries(overlay libtesla common)

get_property(version GLOBAL PROPERTY NXCORD_VERSION)
set_target_properties(overlay PROPERTIES
        APP_TITLE "NXCord"
        APP_AUTHOR "Grarak"
        APP_VERSION "${version}")

add_ovl_target(overlay)