# Third-party notices

## C# conversion

The C# wrapper and NuGet integration are copyright © 2026 DarkSoft and are distributed under the
Zlib license declared by this package.

## raylib-media native core

The packaged `raymedia.dll` is built from raylib-media, copyright (c) 2024 Claudio Z., licensed
under the Zlib license in `LICENSE.md`.

This distribution is altered from the original C project. It adds shared-library export support,
CMake fixes for dynamic Raylib and FFmpeg linking, and an independently written C# interop layer.

Original project: https://github.com/cloudofoz/raylib-media

## Raylib-cs and raylib

Raylib-cs is referenced as a NuGet dependency and supplies the matching raylib runtime. It is not
embedded in this package. See the Raylib-cs package and raylib project for their license notices.

## FFmpeg

FFmpeg binaries are not distributed in this package. `raymedia.dll` dynamically links to FFmpeg 7
shared libraries supplied separately by the application developer. FFmpeg licensing depends on how
those libraries were configured and built; consult the notices shipped with the chosen build.
