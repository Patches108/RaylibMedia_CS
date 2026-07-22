# raylib-media for C#

This folder provides an idiomatic C# API for raylib-media. Your game works with a disposable
`RaylibMedia.MediaStream` object and receives the normal `Raylib_cs.Texture2D` and
`Raylib_cs.AudioStream` types, so there are no unmanaged pointers in game code.

The FFmpeg decoder remains in the native `raymedia` shared library. This is intentional: FFmpeg and
Raylib are native libraries already, and retaining the existing decoder avoids replacing its packet
queueing, seeking, resampling, and audio/video synchronization with a second implementation.

## Requirements

- .NET 8 or newer.
- The `Raylib-cs` 8.0.0 package, included by `RaylibMedia.csproj`. It supplies Raylib 6.0.
- Windows x64 for the 0.1 NuGet preview.
- The FFmpeg 7 runtime DLLs `avcodec-61.dll`, `avformat-61.dll`, `avutil-59.dll`,
  `swresample-5.dll`, and `swscale-8.dll`.

## Install the NuGet package

The current beta is available from
[`RaylibMedia.CS` on NuGet.org](https://www.nuget.org/packages/RaylibMedia.CS/0.1.0-beta.1):

```powershell
dotnet add package RaylibMedia.CS --version 0.1.0-beta.1
```

The package supplies the managed wrapper, `raymedia.dll`, and Raylib-cs. Put the five FFmpeg DLLs
listed above beside the game executable. They must all come from the same 64-bit FFmpeg 7 build;
`ffmpeg.exe` on its own is not sufficient.

If you use the wrapper through NuGet, skip the native build instructions below.

## Add a media file

Place the file at `Media\intro.mp4` in the game project and add this to the game's `.csproj`:

```xml
<ItemGroup>
  <None Update="Media\intro.mp4">
    <CopyToOutputDirectory>PreserveNewest</CopyToOutputDirectory>
    <CopyToPublishDirectory>PreserveNewest</CopyToPublishDirectory>
  </None>
</ItemGroup>
```

The file will be copied to `Media\intro.mp4` under the build and publish output directories. Build
an absolute path from `AppContext.BaseDirectory` when loading it; do not rely on the current working
directory.

## Develop from this source checkout

Add the managed project reference:

```xml
<ItemGroup>
  <ProjectReference Include="path/to/raylib-media/csharp/RaylibMedia/RaylibMedia.csproj" />
</ItemGroup>
```

On Windows x64, the checked-in `runtimes/win-x64/native/raymedia.dll` is copied automatically by
the project reference. Add the five FFmpeg runtime DLLs beside the game executable. Rebuilding the
native decoder is necessary only when changing the C code or targeting another platform.

A native rebuild needs FFmpeg 7.1 development headers/import libraries for `avcodec`, `avformat`,
`avutil`, `swresample`, and `swscale`.

Build `raymedia` against the shared Raylib 6.0 library used by Raylib-cs. Do not statically link a
second Raylib copy into `raymedia`: its separate global graphics/audio state would not be initialized
by your C# game's `Raylib.InitWindow()` and `Raylib.InitAudioDevice()` calls.

Build the native library on Windows by pointing the helper at Raylib's headers and **shared-library
import library**, plus an FFmpeg development build:

```powershell
./csharp/build-native.ps1 `
  -RaylibIncludeDirectory C:\deps\raylib\include `
  -RaylibLibrary C:\deps\raylib\lib\raylib.lib `
  -FfmpegIncludeDirectory C:\deps\ffmpeg\include `
  -FfmpegLibraryDirectory C:\deps\ffmpeg\lib
```

On Linux or macOS, use CMake directly:

```sh
cmake -S . -B build/csharp \
  -DRMEDIA_BUILD_SHARED=ON \
  -DRAYLIB_INCLUDE_DIR=/path/to/raylib/include \
  -DRAYLIB_LIBRARY_RELEASE=/path/to/shared/raylib/library \
  -DFFMPEG_INCLUDE_DIR=/path/to/ffmpeg/include \
  -DFFMPEG_LIBRARY_DIR=/path/to/ffmpeg/lib
cmake --build build/csharp --config Release
```

Place these files beside the game executable (or in its native runtime asset directory):

- `raymedia.dll` on Windows, `libraymedia.so` on Linux, or `libraymedia.dylib` on macOS.
- The FFmpeg shared libraries from the same development build used at link time.
- Raylib-cs copies its own matching Raylib shared library during build.

If the native files cannot be found, .NET throws `DllNotFoundException` and lists the locations it
searched. The architecture must also match the game (`win-x64`, `win-x86`, and so on).

## Minimal game loop

```csharp
using System;
using System.IO;
using Raylib_cs;
using RaylibMedia;

string mediaPath = Path.Combine(AppContext.BaseDirectory, "Media", "intro.mp4");

Raylib.InitWindow(960, 540, "Video");
Raylib.InitAudioDevice();

try
{
    using MediaStream video = MediaStream.Load(mediaPath, MediaLoadFlags.Loop);

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
    // MediaStream is disposed before these calls because of the nested using/try scopes.
    Raylib.CloseAudioDevice();
    Raylib.CloseWindow();
}
```

Initialize the window before loading video and initialize the audio device before loading audio.
Call `Update()` once per frame on the same thread as Raylib. Always dispose media before closing the
audio device or window.

Playback controls are normal C# members:

```csharp
video.Pause();
video.Play();
video.Stop();
video.PositionSeconds = 30.0;
video.Looping = true;
Raylib.SetAudioStreamVolume(video.AudioStream, 0.5f);
```

Use `MediaLoadFlags.NoAudio`, `NoVideo`, or `NoAutoplay` when loading if needed. The
`MediaConfiguration` class exposes the native queue, buffer, delay, channel, and sample-format
settings for streams loaded after a setting changes.

## Managed streams

`System.IO.Stream` input is supported directly. Seekable streams enable FFmpeg seeking; non-seekable
streams work for formats that FFmpeg can read sequentially.

```csharp
using FileStream file = File.OpenRead("packed-video.mp4");
using MediaStream video = MediaStream.Load(file, leaveOpen: true);
```

The default is `leaveOpen: false`, so disposing the media also disposes its input stream.

## Verification

`RaylibMedia.Tests` is a no-test-framework smoke-test executable. Its small C stub exercises the real
P/Invoke ABI: struct returns, C booleans, updates, seeking, configuration, disposal, and managed read
and seek callbacks, without creating a window. The full example in `Example` exercises real playback
once the native Raylib/FFmpeg dependencies are installed.
