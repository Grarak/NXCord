cmake_minimum_required(VERSION 3.1)

project(wslay)

file(GLOB wslay_src ${CMAKE_CURRENT_LIST_DIR}/wslay/lib/*.c)

add_library(wslay STATIC ${wslay_src})

target_compile_definitions(wslay PUBLIC HAVE_ARPA_INET_H WSLAY_VERSION="1.1.0")
target_include_directories(wslay PUBLIC ${CMAKE_CURRENT_LIST_DIR}/wslay/lib/includes)
target_link_libraries(wslay PRIVATE switch::libnx)
