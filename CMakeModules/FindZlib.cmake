include(FindPackageHandleStandardArgs)

if (NOT SWITCH)
    message(FATAL_ERROR "This helper can only be used when cross-compiling for the Switch.")
endif ()

set(ZLIB_INCLUDE_DIR ${DEVKITPRO}/portlibs/switch/include)
set(ZLIB_LIBRARY ${DEVKITPRO}/portlibs/switch/lib/libz.a)

find_package_handle_standard_args(ZLIB DEFAULT_MSG
        ZLIB_INCLUDE_DIR ZLIB_LIBRARY)

mark_as_advanced(ZLIB_INCLUDE_DIR ZLIB_LIBRARY)

if (ZLIB_FOUND)
    set(ZLIB ${ZLIB_INCLUDE_DIR}/..)

    add_library(switch::zlib STATIC IMPORTED GLOBAL)
    set_target_properties(switch::zlib PROPERTIES
            IMPORTED_LOCATION "${ZLIB_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${ZLIB_INCLUDE_DIR}")
endif ()