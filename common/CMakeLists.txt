cmake_minimum_required(VERSION 3.1)

project(common)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB common_src ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)

add_library(common STATIC ${common_src})

target_include_directories(common PUBLIC include)
target_link_libraries(common switch::libnx)
