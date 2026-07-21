# Avoid infinite recursion
if (RAYLIB_FOUND)
    return()
endif()

# Allow user to specify custom include and library paths
find_path(RAYLIB_INCLUDE_DIR NAMES raylib.h HINTS
    ${RAYLIB_INCLUDE_DIR}
    $ENV{RAYLIB_INCLUDE_DIR}
    PATHS
    /usr/include
    /usr/local/include
    /opt/homebrew/include  # macOS Homebrew
)

find_library(RAYLIB_LIBRARY_RELEASE NAMES raylib HINTS
    ${RAYLIB_LIBRARY_DIR}
    $ENV{RAYLIB_LIBRARY_DIR}
    PATHS
    /usr/lib
    /usr/local/lib
    /opt/homebrew/lib
)

find_library(RAYLIB_LIBRARY_DEBUG NAMES raylibd HINTS
    ${RAYLIB_LIBRARY_DIR}
    $ENV{RAYLIB_LIBRARY_DIR}
    PATHS
    /usr/lib
    /usr/local/lib
    /opt/homebrew/lib
)

# Check if we found any valid library
if (RAYLIB_INCLUDE_DIR AND (RAYLIB_LIBRARY_RELEASE OR RAYLIB_LIBRARY_DEBUG))
    add_library(raylib UNKNOWN IMPORTED)

    if (RAYLIB_LIBRARY_RELEASE AND RAYLIB_LIBRARY_DEBUG)
        set_target_properties(raylib PROPERTIES
            IMPORTED_LOCATION_RELEASE "${RAYLIB_LIBRARY_RELEASE}"
            IMPORTED_LOCATION_DEBUG "${RAYLIB_LIBRARY_DEBUG}"
        )
        message(STATUS "✅ Raylib found: Release = ${RAYLIB_LIBRARY_RELEASE}, Debug = ${RAYLIB_LIBRARY_DEBUG}")

    elseif (RAYLIB_LIBRARY_RELEASE)
        set_target_properties(raylib PROPERTIES
            IMPORTED_LOCATION "${RAYLIB_LIBRARY_RELEASE}"
            IMPORTED_LOCATION_RELEASE "${RAYLIB_LIBRARY_RELEASE}"
        )
        message(STATUS "✅ Raylib found (Release only): ${RAYLIB_LIBRARY_RELEASE}")

    elseif (RAYLIB_LIBRARY_DEBUG)
        set_target_properties(raylib PROPERTIES
            IMPORTED_LOCATION "${RAYLIB_LIBRARY_DEBUG}"
            IMPORTED_LOCATION_DEBUG "${RAYLIB_LIBRARY_DEBUG}"
        )
        message(STATUS "✅ Raylib found (Debug only): ${RAYLIB_LIBRARY_DEBUG}")
    endif()

    set_target_properties(raylib PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${RAYLIB_INCLUDE_DIR}"
    )

    set(RAYLIB_FOUND TRUE)
else()
    set(RAYLIB_FOUND FALSE)
    message(FATAL_ERROR "❌ Raylib not found! Provide custom paths using:
    -DRAYLIB_INCLUDE_DIR=/path/to/include
    -DRAYLIB_LIBRARY_DIR=/path/to/lib")
endif()

# Mark variables as advanced to avoid cluttering CMake GUI
mark_as_advanced(RAYLIB_INCLUDE_DIR RAYLIB_LIBRARY_RELEASE RAYLIB_LIBRARY_DEBUG)
