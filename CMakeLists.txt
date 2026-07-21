cmake_minimum_required(VERSION 3.15)
project("raymedia")

# This section configures MSVC (Visual Studio) to enable "Edit and Continue" debugging
if(POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Determine the project's root directory
set(RMEDIA_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# Project configuration options
option(RMEDIA_BUILD_SHARED "Build raymedia as a shared library" OFF)

# Add the CMake module path for custom FindFFMPEG.cmake and FindRaylib.cmake
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules/")

# Find FFmpeg
find_package(FFMPEG REQUIRED)

# Find Raylib
find_package(RAYLIB REQUIRED)

if(NOT RAYLIB_FOUND)
    message(FATAL_ERROR "Raylib not found! Install it or provide custom paths using -DRAYLIB_INCLUDE_DIR and -DRAYLIB_LIBRARY_DIR")
else()
    message(STATUS "Raylib found and linked through the imported raylib target")
endif()

# Set the library type
set(RMEDIA_LIB_TYPE STATIC)
if(RMEDIA_BUILD_SHARED)
    set(RMEDIA_LIB_TYPE SHARED)
endif()

# Create the raymedia library
add_library(${PROJECT_NAME} ${RMEDIA_LIB_TYPE} "${RMEDIA_ROOT_PATH}/src/rmedia.c")

if(RMEDIA_BUILD_SHARED)
    target_compile_definitions(${PROJECT_NAME} PRIVATE RMEDIA_BUILD_SHARED RMEDIA_EXPORTS)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        C_VISIBILITY_PRESET hidden
        VISIBILITY_INLINES_HIDDEN YES
    )
endif()

# Link project dependencies
if(FFMPEG_FOUND AND RAYLIB_FOUND)
    target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/src" ${FFMPEG_INCLUDE_DIRS} ${RAYLIB_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} PUBLIC
        raylib ${FFMPEG_LIBRARIES}
    )

    # Link Windows Multimedia API if on Windows
    if(WIN32)
        target_link_libraries(${PROJECT_NAME} PUBLIC winmm)
    endif()
else()
    message(FATAL_ERROR "FFmpeg or Raylib not found. Please install them or provide the correct paths.")
endif()

# Ensure the C standard is set for MSVC if using CMake version greater than 3.12
if(CMAKE_VERSION VERSION_GREATER 3.12)
    set_property(TARGET raymedia PROPERTY C_STANDARD 99)
endif()

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(FILES "${RMEDIA_ROOT_PATH}/src/raymedia.h" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
