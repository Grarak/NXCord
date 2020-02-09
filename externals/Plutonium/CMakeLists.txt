cmake_minimum_required(VERSION 3.1)

project(plutonium)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

find_package(PlutoniumDeps REQUIRED)

add_library(plutonium STATIC
        ${CMAKE_CURRENT_LIST_DIR}/Plutonium/Plutonium/Source/pu/ui/render/render_SDL2.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Plutonium/Plutonium/Source/pu/ui/render/render_Renderer.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Plutonium/Plutonium/Source/pu/ui/elm/elm_Button.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Plutonium/Plutonium/Source/pu/ui/elm/elm_Toggle.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Plutonium/Plutonium/Source/pu/ui/elm/elm_ProgressBar.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Plutonium/Plutonium/Source/pu/ui/elm/elm_Element.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Plutonium/Plutonium/Source/pu/ui/elm/elm_Rectangle.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Plutonium/Plutonium/Source/pu/ui/elm/elm_TextBlock.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Plutonium/Plutonium/Source/pu/ui/elm/elm_Image.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Plutonium/Plutonium/Source/pu/ui/elm/elm_Menu.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Plutonium/Plutonium/Source/pu/ui/ui_Overlay.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Plutonium/Plutonium/Source/pu/ui/ui_Dialog.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Plutonium/Plutonium/Source/pu/ui/ui_Layout.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Plutonium/Plutonium/Source/pu/ui/ui_Application.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Plutonium/Plutonium/Source/pu/ui/ui_Types.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Plutonium/Plutonium/Source/pu/ui/ui_Container.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Plutonium/Plutonium/Source/pu/ui/extras/extras_Toast.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Plutonium/Plutonium/Source/pu/audio/audio_Sfx.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Plutonium/Plutonium/Source/pu/audio/audio_Music.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Plutonium/Plutonium/Source/pu/pu_String.cpp)

target_include_directories(plutonium
        PRIVATE
        ${DEVKITPRO}/portlibs/switch/include
        PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/Plutonium/Plutonium/Include)
target_link_libraries(plutonium PRIVATE switch::libnx switch::zlib
        PUBLIC ${PLUTONIUM_LIBRARIES} switch::opus)
