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

## Add the required FFmpeg DLLs in Visual Studio

RaylibMedia requires the FFmpeg 7.1 **shared** libraries. An `ffmpeg.exe` file by itself is not
enough, and FFmpeg 8 or a current `master` build has different DLL version numbers.

1. Download
   [`ffmpeg-n7.1-latest-win64-lgpl-shared-7.1.zip`](https://github.com/BtbN/FFmpeg-Builds/releases/download/latest/ffmpeg-n7.1-latest-win64-lgpl-shared-7.1.zip)
   from the [BtbN FFmpeg Builds releases](https://github.com/BtbN/FFmpeg-Builds/releases).
2. Extract the ZIP and open its `bin` folder.
3. In File Explorer, create a folder named `ffmpeg` inside the folder containing your game's
   `.csproj` file.
4. Copy these five files from the extracted `bin` folder into the new `ffmpeg` folder:
   - `avcodec-61.dll`
   - `avformat-61.dll`
   - `avutil-59.dll`
   - `swresample-5.dll`
   - `swscale-8.dll`
5. In Visual Studio, right-click the **game project** in Solution Explorer and select
   **Edit Project File**.
6. Make sure the project targets 64-bit Windows by adding these settings to its existing
   `<PropertyGroup>` if they are not already present:

   ```xml
   <PlatformTarget>x64</PlatformTarget>
   <Prefer32Bit>false</Prefer32Bit>
   <RuntimeIdentifier>win-x64</RuntimeIdentifier>
   ```

7. Add this item group before the closing `</Project>` tag. It copies the DLLs to the root of the
   build and publish folders, beside the game executable and `raymedia.dll`:

   ```xml
   <ItemGroup>
     <None Update="ffmpeg\*.dll">
       <CopyToOutputDirectory>PreserveNewest</CopyToOutputDirectory>
       <CopyToPublishDirectory>PreserveNewest</CopyToPublishDirectory>
       <TargetPath>%(Filename)%(Extension)</TargetPath>
     </None>
   </ItemGroup>
   ```

8. Save the project file, then select **Build > Rebuild Solution**.
9. Open the build output folder, normally
   `bin\Debug\<target-framework>\win-x64\`, and confirm that all five FFmpeg DLLs are in the same
   folder as the game `.exe` and `raymedia.dll`.

Keep the DLLs in the project folder and let MSBuild copy them. Files copied manually into `bin`
can disappear after **Clean** or **Rebuild**. All five DLLs must come from the same FFmpeg build.
When distributing a game, review and comply with the licensing files included in the downloaded
FFmpeg build.

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
