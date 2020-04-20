cmake_minimum_required(VERSION 3.1)

project(http-parser)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(http-parser STATIC ${CMAKE_CURRENT_LIST_DIR}/http-parser/http_parser.c)
set_target_properties(http-parser PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(http-parser
        PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/http-parser)
