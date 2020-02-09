include(FindPackageHandleStandardArgs)

set(PLUTONIUM_DEPS SDL2;
        SDL2_gfx;
        SDL2_image;
        SDL2_mixer;
        SDL2_test;
        SDL2_ttf;
        SDL2main;
        EGL;
        drm_nouveau;
        freetype;
        modplug;
        jpeg;
        png;
        webp;
        mpg123;
        vorbisidec;
        opusfile;
        ogg;
        glapi;
        bz2)

cmake_push_check_state(RESET)
set(PLUTONIUM_LIBRARIES)
set(PLUTONIUM_LIBS_INCLUDE_DIRS ${DEVKITPRO}/portlibs/switch/include)
foreach (DEP IN LISTS PLUTONIUM_DEPS)
    find_library(${DEP}_LIBRARY
            NAMES ${DEP})
    list(APPEND CMAKE_REQUIRED_LIBRARIES ${${DEP}_LIBRARY})
    list(APPEND PLUTONIUM_LIBRARIES ${${DEP}_LIBRARY})
endforeach ()

find_package_handle_standard_args(PLUTONIUM_LIBS
        DEFAULT_MSG
        PLUTONIUM_LIBS_INCLUDE_DIRS PLUTONIUM_LIBRARIES
        )

cmake_pop_check_state()

mark_as_advanced(PLUTONIUM_LIBS_INCLUDE_DIRS PLUTONIUM_LIBRARIES)
