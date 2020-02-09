cmake_minimum_required(VERSION 3.1)

project(simpleiniparser)

file(GLOB simpleiniparser_src ${CMAKE_CURRENT_LIST_DIR}/SimpleIniParser/source/SimpleIniParser/*.cpp)

add_library(simpleiniparser STATIC ${simpleiniparser_src})

target_include_directories(simpleiniparser
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/SimpleIniParser/include/SimpleIniParser
        PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/SimpleIniParser/include)

target_link_libraries(simpleiniparser PRIVATE switch::libnx)
