# Set example sources
set(EXAMPLES
    example_01_basics.c
    example_02_media_player.c
    example_03_multi_stream.c
    example_04_custom_stream.c
)

# Platform-Specific Dependencies
if(WIN32)
    set(PLATFORM_LIBS winmm gdi32 opengl32)
elseif(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(OPENGLLIB OpenGL)
    set(PLATFORM_LIBS ${COCOA_LIBRARY} ${IOKIT_LIBRARY} ${OPENGLLIB})
elseif(UNIX AND NOT APPLE)
    set(PLATFORM_LIBS GL X11 m pthread dl)
endif()

# Create an executable for each example
foreach(EXAMPLE ${EXAMPLES})
    get_filename_component(EXE_NAME ${EXAMPLE} NAME_WE)
    add_executable(${EXE_NAME} ${EXAMPLE})

    # Properly link libraries
    target_link_libraries(${EXE_NAME} PRIVATE raymedia
        debug ${RAYLIB_LIBRARY_DEBUG}
        optimized ${RAYLIB_LIBRARY_RELEASE}
        ${FFMPEG_LIBRARIES}
        ${PLATFORM_LIBS}  # Platform-specific dependencies
    )

    # Include directories
    target_include_directories(${EXE_NAME} PRIVATE 
        "${CMAKE_SOURCE_DIR}/src"
        ${FFMPEG_INCLUDE_DIRS}
        ${RAYLIB_INCLUDE_DIR}
    )
endforeach()

# Copy resources folder to the build directory (DISABLED - User must copy manually)
message(STATUS "⚠️  Manual Action Required: You should copy the 'resources/' folder to the build directory.")
message(STATUS "📌 Note: Some examples require 'resources/videos/trailer.mp4', which is NOT included in the repository. Feel free to use your own video!")

# Optional: If you still want the copy_resources target
# add_custom_target(copy_resources ALL
#     COMMAND ${CMAKE_COMMAND} -E copy_directory
#     "${CMAKE_CURRENT_SOURCE_DIR}/resources" "${CMAKE_BINARY_DIR}/resources"
# )

# foreach(EXAMPLE ${EXAMPLES})
#     get_filename_component(EXE_NAME ${EXAMPLE} NAME_WE)
#     add_dependencies(${EXE_NAME} copy_resources)
# endforeach()
