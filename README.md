# RaylibMedia for C#

RaylibMedia provides FFmpeg-backed video and audio playback for C# games built with Raylib-cs.
It exposes an idiomatic, disposable C# API for loading media, updating decoded frames, controlling
playback, seeking, looping, and reading from managed streams.

This project is currently a preview for Windows x64 and targets .NET 8 or newer.

## How it works

RaylibMedia is a C# wrapper around the native raylib-media decoder. It is not a pure-managed media
decoder:

```text
C# game
  -> RaylibMedia.dll       managed public API
  -> raymedia.dll          native decoder included by the NuGet package
  -> raylib.dll            supplied by Raylib-cs
  -> FFmpeg 7 DLLs         supplied by the game developer
```

Normal package users write only C#. They do not compile C code or run CMake. The native C source
and CMake project are retained so maintainers can audit and rebuild `raymedia.dll` when Raylib or
FFmpeg changes.

## Requirements

- Windows x64 for the current preview.
- .NET 8 or newer.
- Raylib-cs 8.0.0, installed automatically as a package dependency.
- These 64-bit FFmpeg 7 shared libraries beside the game executable:
  - `avcodec-61.dll`
  - `avformat-61.dll`
  - `avutil-59.dll`
  - `swresample-5.dll`
  - `swscale-8.dll`

The FFmpeg DLLs must come from the same build. `ffmpeg.exe` is a command-line application and does
not replace these shared libraries. FFmpeg binaries are not redistributed by this project.

## Installation

After the first NuGet release is published:

```powershell
dotnet add package RaylibMedia --prerelease
```

To develop against this source checkout before publication, add a project reference:

```xml
<ItemGroup>
  <ProjectReference Include="path\to\RaylibMedia_CS\csharp\RaylibMedia\RaylibMedia.csproj" />
</ItemGroup>
```

## Minimal C# example

```csharp
using Raylib_cs;
using RaylibMedia;

Raylib.InitWindow(960, 540, "RaylibMedia");
Raylib.InitAudioDevice();

try
{
    using MediaStream video = MediaStream.Load("intro.mp4", MediaLoadFlags.Loop);

    while (!Raylib.WindowShouldClose())
    {
        video.Update();

        Raylib.BeginDrawing();
        Raylib.ClearBackground(Color.Black);
        Raylib.DrawTexture(video.VideoTexture, 0, 0, Color.White);
        Raylib.EndDrawing();
    }
}
finally
{
    Raylib.CloseAudioDevice();
    Raylib.CloseWindow();
}
```

Initialize the Raylib window before loading video and the audio device before loading audio. Call
`Update()` once per game frame on the Raylib thread. Dispose every `MediaStream` before closing the
audio device or window.

## Playback controls

```csharp
video.Pause();
video.Play();
video.Stop();
video.PositionSeconds = 30.0;
video.Looping = true;
Raylib.SetAudioStreamVolume(video.AudioStream, 0.5f);
```

Use load flags to change initial behavior:

```csharp
using MediaStream silentVideo = MediaStream.Load("intro.mp4", MediaLoadFlags.NoAudio);
using MediaStream pausedVideo = MediaStream.Load("intro.mp4", MediaLoadFlags.NoAutoplay);
using MediaStream audioOnly = MediaStream.Load("music.mp3", MediaLoadFlags.NoVideo);
```

## Loading from a managed stream

Media can be read from a `System.IO.Stream`, including data stored in an archive or another custom
asset source:

```csharp
using FileStream file = File.OpenRead("packed-video.mp4");
using MediaStream video = MediaStream.Load(file, leaveOpen: true);
```

Seekable managed streams support media seeking. Non-seekable streams can be used with formats that
FFmpeg can decode sequentially.

## Repository layout

- `csharp/RaylibMedia` — managed library and NuGet package project.
- `csharp/Example` — runnable C# example.
- `csharp/RaylibMedia.Tests` — native ABI smoke tests.
- `csharp/README.md` — source-build and integration documentation.
- `csharp/NUGET.md` — local tester workflow and owner-only release guide.
- `src` — native raylib-media decoder source used to build `raymedia.dll`.
- `CMakeLists.txt` and `CMakeModules` — maintainer-only native build configuration.

## Building the native runtime

Package consumers can skip this section. Maintainers rebuilding `raymedia.dll` need CMake, shared
Raylib 6 development files, and FFmpeg 7.1 development files:

```powershell
.\csharp\build-native.ps1 `
  -RaylibIncludeDirectory 'D:\path\to\raylib-6.0\src' `
  -RaylibLibrary 'D:\path\to\raylib-6.0\build-shared\raylib\Release\raylib.lib' `
  -FfmpegIncludeDirectory 'D:\path\to\ffmpeg-7.1-shared\include' `
  -FfmpegLibraryDirectory 'D:\path\to\ffmpeg-7.1-shared\lib'
```

The script refreshes the native runtime stored in the NuGet project. It does not add FFmpeg DLLs to
the package.

## Documentation

- [C# integration and source-build guide](csharp/README.md)
- [NuGet testing and owner release guide](csharp/NUGET.md)
- [Package README](csharp/RaylibMedia/README.md)

## License and attribution

The C# wrapper and NuGet integration are distributed under the Zlib license. The native decoder is
based on [raylib-media](https://github.com/cloudofoz/raylib-media), copyright © 2024 Claudio Z., and
is also distributed under the Zlib license.

See [LICENSE.md](LICENSE.md) and
[THIRD-PARTY-NOTICES.md](csharp/RaylibMedia/THIRD-PARTY-NOTICES.md). FFmpeg is not included and has
separate build-dependent licensing terms.
