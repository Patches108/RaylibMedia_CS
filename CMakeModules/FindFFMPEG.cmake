# - Try to find FFmpeg libraries (libavcodec, libavformat, libavutil, libswresample, libswscale)
# Once done, this will define:
#
#  FFMPEG_FOUND - system has FFmpeg
#  FFMPEG_INCLUDE_DIRS - FFmpeg root include directory
#  FFMPEG_LIBRARIES - All required FFmpeg libraries
#
#  Copyright (c) 2008 Andreas Schneider <mail@cynapses.org>
#  Modified for other libraries by Lasse Kärkkäinen <tronic>
#  Modified for Hedgewars by Stepik777
#  Modified for Netgen by Christoph Lehrenfeld (2015) (current version)
#  Modified for Acinerella by Andreas Stöckel (2016) (add SWRESAMPLE)
#  Modifications made for raylib-media by Claudio Z. (@cloudofoz), 2025
#
#  Redistribution and use is allowed according to the terms of the New BSD license.
#

if (FFMPEG_LIBRARIES AND FFMPEG_INCLUDE_DIRS)
  # Already found in cache
  set(FFMPEG_FOUND TRUE)
else()
  find_package(PkgConfig)
  if (PKG_CONFIG_FOUND)
    pkg_check_modules(_FFMPEG libavcodec libavformat libavutil libswresample libswscale)
  endif()

  # Locate FFmpeg root include directory (contains libavcodec/, libavformat/, etc.)
  find_path(FFMPEG_INCLUDE_DIR
    NAMES libavcodec/avcodec.h  # Checking for a key FFmpeg header
    HINTS ${FFMPEG_INCLUDE_DIR} $ENV{FFMPEG_INCLUDE_DIR}
    PATHS /usr/include /usr/local/include /opt/local/include /sw/include
    PATH_SUFFIXES ffmpeg libav
  )

  # Use only the root include directory, avoiding separate paths for each component
  set(FFMPEG_INCLUDE_DIRS ${FFMPEG_INCLUDE_DIR})

  # Locate FFmpeg libraries
  set(_FFMPEG_LIBRARY_HINTS ${FFMPEG_LIBRARY_DIR} $ENV{FFMPEG_LIBRARY_DIR})
  find_library(FFMPEG_LIBAVCODEC NAMES avcodec HINTS ${_FFMPEG_LIBRARY_HINTS} PATHS /usr/lib /usr/local/lib /opt/local/lib /sw/lib)
  find_library(FFMPEG_LIBAVFORMAT NAMES avformat HINTS ${_FFMPEG_LIBRARY_HINTS} PATHS /usr/lib /usr/local/lib /opt/local/lib /sw/lib)
  find_library(FFMPEG_LIBAVUTIL NAMES avutil HINTS ${_FFMPEG_LIBRARY_HINTS} PATHS /usr/lib /usr/local/lib /opt/local/lib /sw/lib)
  find_library(FFMPEG_SWSCALE NAMES swscale HINTS ${_FFMPEG_LIBRARY_HINTS} PATHS /usr/lib /usr/local/lib /opt/local/lib /sw/lib)
  find_library(FFMPEG_SWRESAMPLE NAMES swresample HINTS ${_FFMPEG_LIBRARY_HINTS} PATHS /usr/lib /usr/local/lib /opt/local/lib /sw/lib)

  # Combine all libraries
  set(FFMPEG_LIBRARIES
      ${FFMPEG_LIBAVCODEC}
      ${FFMPEG_LIBAVFORMAT}
      ${FFMPEG_LIBAVUTIL}
      ${FFMPEG_SWSCALE}
      ${FFMPEG_SWRESAMPLE}
  )

  if (FFMPEG_LIBAVCODEC AND FFMPEG_LIBAVFORMAT AND FFMPEG_LIBAVUTIL AND FFMPEG_SWSCALE AND FFMPEG_SWRESAMPLE)
    set(FFMPEG_FOUND TRUE)
  endif()

  if (FFMPEG_FOUND)
    if (NOT FFMPEG_FIND_QUIETLY)
      message(STATUS "Found FFmpeg: ${FFMPEG_LIBRARIES}")
      message(STATUS "FFmpeg includes: ${FFMPEG_INCLUDE_DIRS}")
    endif()
  else()
    if (FFMPEG_FIND_REQUIRED)
      message(FATAL_ERROR "Could not find FFmpeg libraries.")
    endif()
  endif()
endif()
