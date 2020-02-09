include(FindPackageHandleStandardArgs)

if (NOT SWITCH)
    message(FATAL_ERROR "This helper can only be used when cross-compiling for the Switch.")
endif ()

set(SODIUM_INCLUDE_DIR ${DEVKITPRO}/portlibs/switch/include)
set(SODIUM_LIBRARY ${DEVKITPRO}/portlibs/switch/lib/libsodium.a)

find_package_handle_standard_args(SODIUM DEFAULT_MSG
        SODIUM_INCLUDE_DIR SODIUM_LIBRARY)

mark_as_advanced(SODIUM_INCLUDE_DIR SODIUM_LIBRARY)

if (SODIUM_FOUND)
    set(SODIUM ${SODIUM_INCLUDE_DIR}/..)

    add_library(switch::sodium STATIC IMPORTED GLOBAL)
    set_target_properties(switch::sodium PROPERTIES
            IMPORTED_LOCATION "${SODIUM_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${SODIUM_INCLUDE_DIR}")
endif ()