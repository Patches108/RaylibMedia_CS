# RaylibMedia.CS

RaylibMedia brings FFmpeg-backed video and audio playback to C# games built with
[Raylib-cs](https://www.nuget.org/packages/Raylib-cs). It wraps the native raylib-media decoder in
an idiomatic, disposable C# API and exposes normal Raylib-cs textures and audio streams.

> **Preview platform:** version 0.1 ships a Windows x64 native runtime. Linux, macOS, Windows x86,
> and Windows Arm64 are not packaged yet.

## Install

```powershell
dotnet add package RaylibMedia.CS --prerelease
```

The package brings in Raylib-cs 8.0.0 and copies `raymedia.dll` for `win-x64`. You must also put
these **FFmpeg 7** shared libraries beside your game executable:

- `avcodec-61.dll`
- `avformat-61.dll`
- `avutil-59.dll`
- `swresample-5.dll`
- `swscale-8.dll`

The `ffmpeg.exe` command-line program is not a replacement for these libraries. All five DLLs must
come from the same 64-bit FFmpeg build. FFmpeg is not included in this NuGet package.

## Play a video

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
    // Dispose all MediaStream instances before closing Raylib's devices.
    Raylib.CloseAudioDevice();
    Raylib.CloseWindow();
}
```

Initialize the Raylib window before loading video and the audio device before loading audio. Call
`Update()` once per frame on the Raylib thread.

Playback controls are regular C# members:

```csharp
video.Pause();
video.Play();
video.Stop();
video.PositionSeconds = 30.0;
video.Looping = true;
Raylib.SetAudioStreamVolume(video.AudioStream, 0.5f);
```

You can also load from a managed stream:

```csharp
using FileStream file = File.OpenRead("packed-video.mp4");
using MediaStream video = MediaStream.Load(file, leaveOpen: true);
```

## Native requirements

`RaylibMediaRuntime.IsSupportedPlatform` reports whether the current process is supported, and
`RaylibMediaRuntime.RequiredNativeLibraries` lists every required DLL. Your game must target x64;
using `RuntimeIdentifier` `win-x64` is recommended for publishing.

The package and native raylib-media core use the Zlib license. FFmpeg is a separate project with its
own build-dependent licensing terms; consult the license supplied by your chosen FFmpeg build.
