include(FindPackageHandleStandardArgs)

if (NOT SWITCH)
    message(FATAL_ERROR "This helper can only be used when cross-compiling for the Switch.")
endif ()

set(OPUS_INCLUDE_DIR ${DEVKITPRO}/portlibs/switch/include/opus)
set(OPUS_LIBRARY ${DEVKITPRO}/portlibs/switch/lib/libopus.a)

find_package_handle_standard_args(Opus DEFAULT_MSG
        OPUS_INCLUDE_DIR OPUS_LIBRARY)

mark_as_advanced(OPUS_INCLUDE_DIR OPUS_LIBRARY)

if (OPUS_FOUND)
    set(OPUS ${OPUS_INCLUDE_DIR}/..)

    add_library(switch::opus STATIC IMPORTED GLOBAL)
    set_target_properties(switch::opus PROPERTIES
            IMPORTED_LOCATION "${OPUS_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${OPUS_INCLUDE_DIR}")
endif ()