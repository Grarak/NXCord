cmake_minimum_required(VERSION 3.1)

project(nxcord-all)

message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}.")

string(TIMESTAMP TODAY "%Y%m%d")
set(VERSION "0.4.1-${TODAY}")
set_property(GLOBAL PROPERTY NXCORD_VERSION ${VERSION})
add_definitions(-DNXCORD_VERSION="${VERSION}")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/CMakeModules)

include(SwitchTools)

find_package(Libnx REQUIRED)
find_package(MbedTLS REQUIRED)
find_package(Opus REQUIRED)
find_package(Sodium REQUIRED)
find_package(Zlib REQUIRED)

include(externals/Atmosphere-libs/CMakeLists.txt)
include(externals/http-parser/CMakeLists.txt)
include(externals/libtesla/CMakeLists.txt)
include(externals/Plutonium/CMakeLists.txt)
include(externals/SimpleIniParser/CMakeLists.txt)
include(externals/sleepy-discord/sleepy_discord/CMakeLists.txt)
include(externals/wslay/CMakeLists.txt)
include(common/CMakeLists.txt)
include(nxcord/CMakeLists.txt)
include(client/CMakeLists.txt)
include(overlay/CMakeLists.txt)
include(sysmodule/CMakeLists.txt)
