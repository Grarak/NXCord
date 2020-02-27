cmake_minimum_required(VERSION 3.1)

project(libtesla)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(libtesla STATIC ${CMAKE_CURRENT_LIST_DIR}/libtesla/include/tesla.hpp)
set_target_properties(libtesla PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(libtesla
        PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/libtesla/include)
target_link_libraries(libtesla switch::libnx)
