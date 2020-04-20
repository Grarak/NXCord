cmake_minimum_required(VERSION 3.1)

project(nxcord)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB nxcord_src ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)

add_library(nxcord STATIC ${nxcord_src})

target_include_directories(nxcord PUBLIC include)
target_link_libraries(nxcord PUBLIC wslay switch::libnx ${MBEDTLS_LIBRARIES}
        simpleiniparser sleepy-discord common ${ZLIB_LIBRARY} http-parser)
